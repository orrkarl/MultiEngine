#pragma once

#include <vulkan/vulkan.h>

#include "AutoGeneratedKernelResources.h"

#include "shared/compatibility.h"

namespace vkr {
namespace gpu {

namespace triangle_setup {

VkShaderModule createModule(VkResult& status, VkDevice dev, const VkAllocationCallbacks* allocator = nullptr);
VkDescriptorSetLayout createLayout(VkResult& status, VkDevice dev,
                                   const VkAllocationCallbacks* allocator = nullptr);
VkPushConstantRange describePushConstants();

} // triangle_setup

} // gpu
} // vkr
