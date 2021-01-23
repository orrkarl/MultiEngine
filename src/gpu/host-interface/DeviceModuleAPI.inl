namespace vkr {
namespace gpu {

template<typename APILayout>
DeviceModuleAPI<APILayout>::DeviceModuleAPI()
    : m_code(VK_NULL_HANDLE)
    , m_argsDesc{}
    , m_runnerDesc(VK_NULL_HANDLE) {
    // This code is probably redundant (as VK_NULL_HANDLE = 0), it's here just to be explicit
    for (auto& arg : m_argsDesc) {
        arg = VK_NULL_HANDLE;
    }
}

template<typename APILayout>
VkResult DeviceModuleAPI<APILayout>::init(VkDevice dev, const VkAllocationCallbacks *allocator) {
    VkShaderModuleCreateInfo codeInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                             nullptr,
                                             0,
                                             APILayout::MODULE_SIZE * sizeof(uint32_t),
                                             APILayout::MODULE_DATA };

    VkResult status = vkCreateShaderModule(dev, &codeInfo, allocator, &m_code);
    if (status != VK_SUCCESS) {
        return status;
    }

    uint32_t currentSet = 0;
    for (size_t i = 0; i < APILayout::SETS.size(); ++i) {
        VkDescriptorSetLayoutCreateInfo argSetInfo {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, APILayout::SETS[i], &APILayout::BINDINGS[currentSet]
        };
        status = vkCreateDescriptorSetLayout(dev, &argSetInfo, allocator, &m_argsDesc[i]);
        if (status != VK_SUCCESS) {
            for (size_t j = i; j > 0; --j) {
                vkDestroyDescriptorSetLayout(dev, m_argsDesc[j], allocator);
                m_argsDesc[j] = VK_NULL_HANDLE;
            }
            vkDestroyShaderModule(dev, m_code, allocator);
            m_code = VK_NULL_HANDLE;
            return status;
        }

        currentSet += APILayout::SETS[i];
    }

    VkPipelineLayoutCreateInfo pipelineDesc { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                              nullptr,
                                              0,
                                              static_cast<uint32_t>(m_argsDesc.size()),
                                              m_argsDesc.data(),
                                              static_cast<uint32_t>(APILayout::PUSH_ARGS.size()),
                                              APILayout::PUSH_ARGS.data()};
    status = vkCreatePipelineLayout(dev, &pipelineDesc, allocator, &m_runnerDesc);
    if (status != VK_SUCCESS) {
        for (auto it = m_argsDesc.rbegin(); it != m_argsDesc.rend(); ++it) {
            vkDestroyDescriptorSetLayout(dev, *it, allocator);
            *it = VK_NULL_HANDLE;
        }
        vkDestroyShaderModule(dev, m_code, allocator);
        m_code = VK_NULL_HANDLE;
        return status;
    }

    return VK_SUCCESS;
}

template<typename APILayout>
void DeviceModuleAPI<APILayout>::destroy(VkDevice dev, const VkAllocationCallbacks *allocator) {
    vkDestroyPipelineLayout(dev, m_runnerDesc, allocator);
    m_runnerDesc = VK_NULL_HANDLE;
    for (auto it = m_argsDesc.rbegin(); it != m_argsDesc.rend(); ++it) {
        vkDestroyDescriptorSetLayout(dev, *it, allocator);
        *it = VK_NULL_HANDLE;
    }
    vkDestroyShaderModule(dev, m_code, allocator);
    m_code = VK_NULL_HANDLE;
}

template<typename APILayout>
typename DeviceModuleAPI<APILayout>::ArgumentSetsLayout DeviceModuleAPI<APILayout>::describeArguments() {
    return m_argsDesc;
}

template<typename APILayout>
VkComputePipelineCreateInfo DeviceModuleAPI<APILayout>::describeRunner() {
    return { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
             nullptr,
             0,
             VkPipelineShaderStageCreateInfo { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                               nullptr,
                                               0,
                                               VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
                                               m_code,
                                               "main",
                                               nullptr },
             m_runnerDesc,
             VK_NULL_HANDLE,
             0 };
}

}
}