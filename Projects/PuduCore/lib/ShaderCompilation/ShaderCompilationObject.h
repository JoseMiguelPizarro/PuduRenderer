#pragma once
#include <unordered_map>

#include "DescriptorSetLayoutCollection.h"
#include "GPUEnums.h"
#include "VertexLayout.h"
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
        std::optional<ShaderKernel*> GetKernel(const char* name);
        void AddKernel(const char* name, ShaderKernel& kernel);
        std::vector<VkPushConstantRange>* GetPushConstantRanges() { return &m_pushConstantRanges; }
        PushConstantInfo* GetPushConstantsInfo();
        std::vector<ConstantBufferInfo>* GetConstantBuffers() { return &m_constantBuffers; }
        std::vector<ConstantBufferInfo>* GetPushConstantsBuffersInfo() { return &m_pushConstants; }
        void SetBuffersToAllocate(const std::vector<ConstantBufferInfo>& buffers) { m_constantBuffers = buffers; }
        void SetPushConstants(const ConstantBufferInfo* buffers, Size count);
        BlendState GetBlendState() const { return m_blendState; }
        CullMode GetCullMode() const { return m_cullMode; }
        bool OverridePipelineState() const { return m_overridesPipelineState; }

        UPtr<std::vector<fs::path>> GetDependencies()
        {
            return std::make_unique<std::vector<fs::path>>(m_dependencies);
        }

        VertexLayout* GetVertexLayout(){return &m_vertexLayout;}

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
        VertexLayout m_vertexLayout;
        CullMode m_cullMode;
        bool m_overridesPipelineState = false;
    };
}
