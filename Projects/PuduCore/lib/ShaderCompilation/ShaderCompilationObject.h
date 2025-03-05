#pragma once
#include <unordered_map>

#include "DescriptorSetLayoutData.h"
#include "ShaderCompilation/ShaderCompilationObject.h"

namespace Pudu{
    struct ShaderKernel {
        const u32* code;
        size codeSize;
    };

    struct ConstantBufferInfo
    {
        size offset = 0;
        size stride = 0;
        size size = 0;
        DescriptorSetLayout descriptorSetLayout;
        void PushElement(size_t size);
    };

class ShaderCompilationObject {
    public:
        DescriptorSetLayoutsData descriptorsData;
        ShaderKernel* GetKernel(const char* name) { return &m_kernelsByName[name]; }
        void AddKernel(const char* name, ShaderKernel& kernel);
        std::vector<VkPushConstantRange>* GetPushConstantRanges() { return &m_pushConstantRanges;}
        std::vector<ConstantBufferInfo>* GetConstantBuffers(){ return &m_constantBuffers;}
        void SetBuffersToAllocate(const std::vector<ConstantBufferInfo>& buffers) {m_constantBuffers = buffers;}

    private:
        friend class DescriptorsBuilder;
        std::unordered_map<std::string, ShaderKernel> m_kernelsByName;
        std::vector<VkPushConstantRange> m_pushConstantRanges;
        std::vector<ConstantBufferInfo> m_constantBuffers;
};
}



