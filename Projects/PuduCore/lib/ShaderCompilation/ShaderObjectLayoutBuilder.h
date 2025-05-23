#pragma once
#include <iostream>
#include <slang.h>
#include <slang-com-ptr.h>

#include "Logger.h"
#include "ShaderCompilationObject.h"
#include "ShaderObject.h"
#include "DescriptorSetLayoutCollection.h"

namespace Pudu
{
    using namespace slang;

    // ### Access Paths

    struct AccessPathNode
    {
        slang::VariableLayoutReflection* variableLayout = nullptr;
        AccessPathNode* parent = nullptr;
    };

    struct Binding
    {
        size index = -1;

        size PushIndex()
        {
            index++;
            return index;
        }
    };

    struct AccessPath
    {
        AccessPath() = default;

        bool valid = false;
        AccessPathNode* leaf = nullptr;
        ConstantBufferInfo* rootBufferInfo = nullptr;
        DescriptorSetLayoutInfo* descriptorSetLayout = nullptr;
        size_t setIndex = -1;
        Binding* cumulativeOffset = nullptr;
        ShaderNode* shaderNode = nullptr;
        //Shader node that points to the current deepest buffer
        ShaderNode* rootBufferShaderNode = nullptr;


        bool isPushConstant = false;
        bool isContainerStructDefinition = false;

        void Print() const;
    };

    struct ExtendedAccessPath : AccessPath
    {
        ExtendedAccessPath(AccessPath const& base, slang::VariableLayoutReflection* variableLayout)
            : AccessPath(base)
        {
            if (!valid)
                return;

            ASSERT(variableLayout != nullptr, "Variable layout cannot be null");

            element.variableLayout = variableLayout;
            element.parent = leaf;

            leaf = &element;
        }

        AccessPathNode element;
    };


    struct Offset
    {
        u16 value = 0;
        u16 space = 0;
    };

    struct ShaderLayoutBuilderContext
    {
        ShaderCompilationObject* shaderCompilationObject = nullptr;
        ShaderNode* shaderLayout = nullptr;
        size_t constantBufferSize = 0;
        ConstantBufferInfo* PushConstantBufferInfo();
        ConstantBufferInfo* PushPushConstantsBufferInfo();
        std::vector<ConstantBufferInfo>& GetPushConstants();
        std::vector<ConstantBufferInfo>* GetConstantBufferInfos();

        Size PushSetIndex() { return ++m_setIndex; }
        Size getSetIndex() const { return m_setIndex; }
        void PushBinding(const DescriptorBinding& binding) const;

        ShaderLayoutBuilderContext();

    private:
        const Size MAX_BUFFERS_COUNT = 32;
        size_t m_setIndex = -1;
        std::vector<ConstantBufferInfo> m_constantBuffers;
        std::vector<ConstantBufferInfo> m_pushConstants;
    };

    struct ShaderObjectLayoutBuilder
    {
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        u32 m_bindingIndex = 0;
        Slang::ComPtr<IGlobalSession> m_globalSession;

        void ParseShaderProgramLayout(slang::ProgramLayout* programLayout,
                                      ShaderCompilationObject& outCompilationObject);

    private:
        void ParseVariableTypeLayout(TypeLayoutReflection* typeLayoutReflection, ShaderLayoutBuilderContext* context,
                                     AccessPath accessPath);
        void ParseVariableLayout(VariableLayoutReflection* varLayout, ShaderLayoutBuilderContext* context,
                                 AccessPath accessPath);
        void ParseVariableOffsets(VariableLayoutReflection* varLayout, ShaderLayoutBuilderContext* context,
                                  AccessPath accessPath);
        void ParseScope(slang::VariableLayoutReflection* scopeVarLayout, ShaderLayoutBuilderContext* context,
                        AccessPath accessPath);

    private:
        u32 m_indentation = 0;
    };
} // Pudu
