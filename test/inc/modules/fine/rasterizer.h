#pragma once

#include <inc/includes.h>

#include <base/Module.h>
#include <kernels/base.cl.h>
#include <kernels/fine_rasterizer.cl.h>
#include <base/Buffer.h>
#include <rendering/Render.h>
#include <pipeline/BinRasterizer.h>
#include <pipeline/FineRasterizer.h>
#include "fine_utils.h"

using namespace nr;
using namespace nr::__internal;
using namespace testing;

void validateFragment(const Fragment& fragment, const ScreenDimension& screenDim, const RawColorRGB* color, const NRfloat* depth)
{
    const NRuint idx = index_from_screen(fragment.position, screenDim);

    ASSERT_EQ(fragment.color, color[idx]);
    ASSERT_EQ(fragment.depth, depth[idx]);
}

TEST(Fine, Rasterizer)
{
    Error error = Error::NO_ERROR;
    cl_int err = CL_SUCCESS;

    const NRuint dim = 5;

    const ScreenDimension screenDim = { 20, 10 };
    const NRuint totalScreenSize = screenDim.width * screenDim.height;

    const BinQueueConfig config = { 5, 5, 10 };
    const NRuint binCountX = ceil(((NRfloat) screenDim.width) / config.binWidth);
    const NRuint binCountY = ceil(((NRfloat) screenDim.height) / config.binHeight);
    const NRuint totalBinCount = binCountX * binCountY;
    const NRuint totalWorkGroupCount = 4;
    const NRuint totalBinQueuesSize = totalWorkGroupCount * totalBinCount * (config.queueSize + 1); 

    const NRfloat expectedDepth = 0.5;

    const NRuint triangleCount = 3 * totalBinCount;
    std::unique_ptr<Triangle<dim>[]> h_triangles(new Triangle<dim>[triangleCount]);
    std::unique_ptr<NRuint[]> h_binQueues(new NRuint[totalBinQueuesSize]);

    std::unique_ptr<RawColorRGB[]> h_colorBuffer(new RawColorRGB[totalScreenSize]);
    std::unique_ptr<NRfloat[]> h_depthBuffer(new NRfloat[totalScreenSize]);

    Fragment expected;
    expected.color = RED;

    for (NRuint i = 0; i < totalScreenSize; ++i)
    {
        h_colorBuffer[i] = { 0, 0, 0 };
        h_depthBuffer[i] = 1.1;
    }
    
    fillTriangles<dim>(screenDim, config, totalWorkGroupCount, expectedDepth, 256, h_triangles.get(), h_binQueues.get());

    const NRchar options_fmt[] = "-cl-std=CL2.0 -Werror -D _DEBUG -D _TEST_FINE -D RENDER_DIMENSION=%d";
    NRchar options[sizeof(options_fmt) * 2];
    memset(options, 0, sizeof(options));
    sprintf(options, options_fmt, dim);

    Module code({clcode::base, clcode::fine_rasterizer}, options, &err);
    ASSERT_TRUE(isSuccess(err));

    auto testee = code.makeKernel<FineRasterizerParams>("fine_rasterize", &err);
    ASSERT_TRUE(isSuccess(err));

    auto q = cl::CommandQueue::getDefault(&err);
    ASSERT_TRUE(isSuccess(err));

    FrameBuffer frame;
    frame.color = Buffer(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, totalScreenSize * sizeof(RawColorRGB), h_colorBuffer.get(), &error);
    ASSERT_TRUE(isSuccess(error));
    frame.depth = Buffer(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, totalScreenSize * sizeof(NRfloat), h_depthBuffer.get(), &error);
    ASSERT_TRUE(isSuccess(error));

    Buffer d_triangles(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, triangleCount * sizeof(Triangle<dim>), h_triangles.get(), &error);
    ASSERT_TRUE(isSuccess(error));
    Buffer d_binQueues(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, totalBinQueuesSize * sizeof(NRuint), h_binQueues.get(), &error);
    ASSERT_TRUE(isSuccess(error));

    testee.params.triangleData = d_triangles;
    testee.params.dim = screenDim;
    testee.params.binQueueConfig = config;
    testee.params.binQueues = d_binQueues;
    testee.params.workGroupCount = totalWorkGroupCount;
    testee.params.frameBuffer = frame;

    testee.global = cl::NDRange(binCountX, binCountY);
    testee.local  = cl::NDRange(binCountX, binCountY);

    ASSERT_TRUE(isSuccess(testee(q)));
    ASSERT_TRUE(isSuccess(q.enqueueReadBuffer(frame.color, CL_FALSE, 0, totalScreenSize * sizeof(RawColorRGB), h_colorBuffer.get())));
    ASSERT_TRUE(isSuccess(q.enqueueReadBuffer(frame.depth, CL_FALSE, 0, totalScreenSize * sizeof(NRfloat), h_depthBuffer.get())));
    ASSERT_TRUE(isSuccess(q.finish()));

    printf("Color: \n");
    for (NRuint y = 0; y < screenDim.height; ++y)
    {
        for (NRuint x = 0; x < screenDim.width; ++x)
        {
            auto c = h_colorBuffer[y * screenDim.width + x];
            printf("(%.3d,%d,%d) ", (NRuint) c.r, (NRuint) c.g, (NRuint) c.b);
        }
        printf("\n");
    }

    printf("Depth: \n");
    for (NRuint y = 0; y < screenDim.height; ++y)
    {
        for (NRuint x = 0; x < screenDim.width; ++x)
        {
            printf("%.3f ", h_depthBuffer[y * screenDim.width + x]);
        }
        printf("\n");
    }

    printf("Bin queues:\n");
    for (NRuint g = 0; g < totalWorkGroupCount; ++g)
    {
        printf("Group %d:\n", g);
        for (NRuint y = 0; y < binCountY; ++y)
        {
            for (NRuint x = 0; x < binCountX; ++x)
            {
                printf("Queue for bin (%d, %d):\t", x, y);
                for (NRuint i = 0; i < config.queueSize + 1; ++i)
                {
                    printf("%.3d ", h_binQueues[g * totalBinCount * (config.queueSize + 1) + (y * binCountX + x) * (config.queueSize + 1) + i]);
                }
                printf("\n");
            }
        }
    }

    for (NRuint y = 0; y < screenDim.height; y += config.binHeight)
    {
        for (NRuint x = 0; x < screenDim.width; x += config.binWidth)
        {
            expected.position.x = x;
            expected.position.y = y;
            expected.depth = expectedDepth;
            validateFragment(expected, screenDim, h_colorBuffer.get(), h_depthBuffer.get());
        }
    }
}