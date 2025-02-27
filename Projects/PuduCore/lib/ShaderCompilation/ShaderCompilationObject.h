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
        size offset;
        size stride;
        size size;
        DescriptorSetLayout descriptorSetLayout;
    };

class ShaderCompilationObject {
    public:
        DescriptorSetLayoutsData descriptorsData;
        ShaderKernel* GetKernel(const char* name) { return &m_kernelsByName[name]; }
        void AddKernel(const char* name, ShaderKernel& kernel);
        std::vector<VkPushConstantRange>* GetPushConstantRanges() { return &m_pushConstantRanges;}
        std::vector<ConstantBufferInfo>* GetConstantBuffers(){ return &m_constantBuffers;}

    private:
        friend class DescriptorsBuilder;
        std::unordered_map<std::string, ShaderKernel> m_kernelsByName;
        std::vector<VkPushConstantRange> m_pushConstantRanges;
        std::vector<ConstantBufferInfo> m_constantBuffers;
};
}



