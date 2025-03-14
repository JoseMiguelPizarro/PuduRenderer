#pragma once
#include <unordered_map>

#include "DescriptorSetLayoutCollection.h"
#include "ShaderLayout.h"
#include "ShaderCompilation/ShaderCompilationObject.h"
#include "Resources/ConstantBufferInfo.h"

namespace Pudu{
    struct ShaderKernel {
        const u32* code;
        size codeSize;
    };

class ShaderCompilationObject {
    public:
        DescriptorSetLayoutsCollection descriptorsData;
        ShaderKernel* GetKernel(const char* name) { return &m_kernelsByName[name]; }
        void AddKernel(const char* name, ShaderKernel& kernel);
        std::vector<VkPushConstantRange>* GetPushConstantRanges() { return &m_pushConstantRanges;}
        std::vector<ConstantBufferInfo>* GetConstantBuffers(){ return &m_constantBuffers;}
        std::vector<ConstantBufferInfo>* GetPushConstantsBuffersInfo(){ return &m_pushConstants;}
        void SetBuffersToAllocate(const std::vector<ConstantBufferInfo>& buffers) {m_constantBuffers = buffers;}
        void SetPushConstants(const std::vector<ConstantBufferInfo>& buffers);
        ShaderNode* GetShaderLayout();



    private:
        friend class ShaderObjectLayoutBuilder;
        std::unordered_map<std::string, ShaderKernel> m_kernelsByName;
        std::vector<VkPushConstantRange> m_pushConstantRanges;
        std::vector<ConstantBufferInfo> m_constantBuffers;
        std::vector<ConstantBufferInfo> m_pushConstants;
        ShaderNode m_shaderLayout;
};
}



