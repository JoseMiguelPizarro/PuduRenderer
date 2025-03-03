#pragma once
#include <iostream>
#include <slang.h>
#include <slang-com-ptr.h>

#include "Logger.h"
#include "ShaderCompilationObject.h"
#include "ShaderObject.h"

namespace Pudu
{
    using namespace slang;

    struct CumulativeOffset
    {
        u32 value = 0;
        u32 space = 0;
    };

    // ### Access Paths

    struct AccessPathNode
    {
        slang::VariableLayoutReflection* variableLayout = nullptr;
        AccessPathNode* parent = nullptr;
    };

    struct AccessPath
    {
        AccessPath() = default;

        bool valid = false;
        AccessPathNode* deepestConstantBufer = nullptr;
        AccessPathNode* deepestParameterBlock = nullptr;
        AccessPathNode* leaf = nullptr;
        ConstantBufferInfo* rootBufferInfo;
        size_t setIndex = -1;


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
        size_t setIndex = -1;
        ShaderCompilationObject* shaderCompilationObject = nullptr;
        size_t constantBufferSize = 0;
        ConstantBufferInfo* PushConstantBufferInfo();

    private:
        std::vector<ConstantBufferInfo> m_constantBuffers;
    };

    struct ShaderObjectLayoutBuilder
    {
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        u32 m_bindingIndex = 0;
        void AddBindingsForParameterBlock(
            slang::TypeLayoutReflection* typeLayout, DescriptorSetLayoutsData& layoutsData);
        void ParseShaderProgramLayout(slang::ProgramLayout* programLayout,
                                      ShaderCompilationObject& outCompilationObject);
        void ParseVariableTypeLayout(TypeLayoutReflection* typeLayoutReflection, ShaderLayoutBuilderContext* context,AccessPath accessPath);
        void ParseVariableLayout(VariableLayoutReflection* varLayout, ShaderLayoutBuilderContext* context, AccessPath accessPath);
        void ParseVariableOffsets(VariableLayoutReflection* varLayout, ShaderLayoutBuilderContext* context, AccessPath accessPath);
        void ParseScope(slang::VariableLayoutReflection* scopeVarLayout, ShaderLayoutBuilderContext* context, AccessPath accessPath);
        void addBindingsFrom(
            slang::TypeLayoutReflection* typeLayout,
            u32 descriptorCount);

        static CumulativeOffset CalculateCumulativeOffset(
            slang::VariableLayoutReflection* variableLayout,
            slang::ParameterCategory layoutUnit,
            AccessPath accessPath);

        Slang::ComPtr<IGlobalSession> m_globalSession;

    private:
        u32 m_indentation = 0;
    };
} // Pudu
