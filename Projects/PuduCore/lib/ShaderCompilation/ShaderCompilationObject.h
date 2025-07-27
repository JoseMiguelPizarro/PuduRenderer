#pragma once
#include <unordered_map>

#include "DescriptorSetLayoutCollection.h"
#include "GPUEnums.h"
#include "ShaderLayout.h"
#include "ShaderCompilation/ShaderCompilationObject.h"
#include "Resources/ConstantBufferInfo.h"
#include "Resources/Resources.h"

namespace Pudu
{
    enum class ShaderCompilationResult
    {
        Success,
        Warning,
        Failed,
    };

    struct ShaderKernel
    {
        const u32* code;
        size codeSize;
    };

    class ShaderCompilationObject
    {
    public:
        DescriptorSetLayoutsCollection descriptorsData;
        ShaderCompilationResult result;
        ShaderKernel* GetKernel(const char* name) { return &m_kernelsByName[name]; }
        void AddKernel(const char* name, ShaderKernel& kernel);
        std::vector<VkPushConstantRange>* GetPushConstantRanges() { return &m_pushConstantRanges; }
        PushConstantInfo* GetPushConstantsInfo();
        std::vector<ConstantBufferInfo>* GetConstantBuffers() { return &m_constantBuffers; }
        std::vector<ConstantBufferInfo>* GetPushConstantsBuffersInfo() { return &m_pushConstants; }
        void SetBuffersToAllocate(const std::vector<ConstantBufferInfo>& buffers) { m_constantBuffers = buffers; }
        void SetPushConstants(const std::vector<ConstantBufferInfo>& buffers);
        BlendState GetBlendState() const { return m_blendState; }
        CullMode GetCullMode() const { return m_cullMode; }

        UPtr<std::vector<fs::path>> GetDependencies()
        {
            return std::make_unique<std::vector<fs::path>>(m_dependencies);
        }

    private:
        friend class ShaderObjectLayoutBuilder;
        friend class ShaderCompiler;

        std::unordered_map<std::string, ShaderKernel> m_kernelsByName;
        std::vector<VkPushConstantRange> m_pushConstantRanges;
        std::vector<ConstantBufferInfo> m_constantBuffers;
        std::vector<ConstantBufferInfo> m_pushConstants;
        std::vector<fs::path> m_dependencies;
        PushConstantInfo m_pushConstantsInfo;
        BlendState m_blendState;
        CullMode m_cullMode;
    };
}
