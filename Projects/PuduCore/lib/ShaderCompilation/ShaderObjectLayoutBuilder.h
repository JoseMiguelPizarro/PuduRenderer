#pragma once
#include <slang.h>
#include <slang-com-ptr.h>

#include "ShaderCompilationObject.h"
#include "ShaderObject.h"

namespace Pudu {

    using namespace slang;

    struct CumulativeOffset
    {
        size_t value = 0;
        size_t space = 0;
    };

    // ### Access Paths

    struct AccessPathNode
    {
        slang::VariableLayoutReflection* variableLayout = nullptr;
        AccessPathNode* outer = nullptr;
    };

    struct AccessPath
    {
        AccessPath() {}

        bool valid = false;
        AccessPathNode* deepestConstantBufer = nullptr;
        AccessPathNode* deepestParameterBlock = nullptr;
        AccessPathNode* leaf = nullptr;
    };

    struct ExtendedAccessPath : AccessPath
    {
        ExtendedAccessPath(AccessPath const& base, slang::VariableLayoutReflection* variableLayout)
            : AccessPath(base)
        {
            if (!valid)
                return;

            element.variableLayout = variableLayout;
            element.outer = leaf;

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
        AccessPath* accessPath = nullptr;
        ShaderCompilationObject* shaderCompilationObject = nullptr;
    };

    struct ShaderObjectLayoutBuilder
    {
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        u32 m_bindingIndex = 0;
        void AddBindingsForParameterBlock(
            slang::TypeLayoutReflection* typeLayout, DescriptorSetLayoutsData& layoutsData);
        void ParseShaderProgramLayout(slang::ProgramLayout* programLayout,ShaderCompilationObject& outCompilationObject);
        void ParseVariableTypeLayout(TypeLayoutReflection* typeLayoutReflection, ShaderLayoutBuilderContext& context);
        void ParseVariableLayout(VariableLayoutReflection* varLayout, ShaderLayoutBuilderContext& context);
        void ParseVariableOffsets(VariableLayoutReflection* varLayout, ShaderLayoutBuilderContext& context);
        void ParseScope(slang::VariableLayoutReflection* scopeVarLayout,ShaderLayoutBuilderContext& context);
        void addBindingsFrom(
            slang::TypeLayoutReflection* typeLayout,
            u32 descriptorCount);

        CumulativeOffset CalculateCumulativeOffset(
        slang::VariableLayoutReflection* variableLayout,
        slang::ParameterCategory layoutUnit,
        AccessPath accessPath);
        void ParseTypeKind(TypeLayoutReflection* typeLayoutReflection, ShaderLayoutBuilderContext& context);

        Slang::ComPtr<IGlobalSession> m_globalSession;
    };
} // Pudu

