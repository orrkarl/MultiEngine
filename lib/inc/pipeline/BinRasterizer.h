#pragma once

#include "../general/predefs.h"
#include "../base/Buffer.h"
#include "../base/Kernel.h"
#include "../rendering/RenderState.h"

namespace nr
{

namespace __internal
{

class BinRasterizerParams
{
public:
    cl_int load(cl::Kernel kernel, const BinRasterizerParams& self);

    cl_int allocateBinQueues(const NRuint workGroupCount, const ScreenDimension& dim, const NRuint& binWidth, const NRuint& binHeight, const NRuint binQueueSize)
    {
        m_binQueues.resize(CL_MEM_READ_WRITE, workGroupCount * getBinCount(dim, binWidth, binHeight) * binQueueSize * sizeof(NRuint));
        m_binQueueSize = binQueueSize;
        m_dim = dim;

        m_binWidth = binWidth;
        m_binHeight = binHeight;
    }


    void configureSimplexBuffer(Buffer data, const NRuint& count)
    {
        m_simplexData = data;
        m_simplexCount = count;
    }

private:
    NRuint getBinCount(const ScreenDimension& dim, const NRuint& binWidth, const NRuint& binHeight)
    {
        NRuint xCount = (NRuint) ceil(((NRfloat) dim.width) / binWidth);
        NRuint yCount = (NRuint) ceil(((NRfloat) dim.height) / binHeight);
        return xCount * yCount;
    }

    // Screen Dimensions
    ScreenDimension m_dim;

    // Bin Queues Data
    NRuint m_binQueueSize;
    Buffer m_binQueues;

    // Bin data
    NRuint m_binWidth;
    NRuint m_binHeight;

    // Simplex data
    Buffer m_simplexData;
    NRuint m_simplexCount;
};

typedef Kernel<BinRasterizerParams> BinRasterizer;

}

}