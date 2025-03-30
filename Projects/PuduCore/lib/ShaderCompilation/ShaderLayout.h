#pragma once
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

#include "DescriptorSetLayoutInfo.h"
#include "../PuduCore.h"

namespace Pudu
{
    enum ShaderNodeType
    {
        Root,
        //THIS IS RESERVED FOR CBUFFER USUALLY AUTOMATICALLY ALLOCATED
        CBuffer,
        Buffer,
        Array,
        Resource,
        Uniform,
        PushConstant,
        ParameterBlock,
        Struct,
    };

    static std::map<ShaderNodeType, const char*> SHADER_NODE_TYPE_NAMES = {
        {Root, "Root"},
        {CBuffer, "CBuffer"},
        {Array, "Array"},
        {Resource, "Resource"},
        {Uniform, "Uniform"},
        {PushConstant, "PushConstant"},
        {ParameterBlock, "ParameterBlock"},
        {Struct, "Struct"}
    };

    struct ShaderNodeHandle
    {
        Size index;
    };

    constexpr Size MAX_CHILDREN = 64;
    static const char* ROOT_NAME = "Root";

    struct ShaderNode
    {
        std::string name;
        u32 offset = 0;
        Size size = 0;
        Size elementCount = 0;

        u32 bindingIndex = 0;
        u32 setIndex = 0;
        u32 setId = 0;

        ShaderNodeType type;

        Size childCount = 0;
        DescriptorBinding binding;
        std::string scope;

        //Pointer to the final container this shader node will part of. ie CBuffer->Struct->floatValue for floatValue its container is CBuffer while its parent is Struct
        ShaderNode* parentContainer = nullptr;
        ShaderNode* parent = nullptr;


        ShaderNode()
        {
            children.reserve(MAX_CHILDREN);
        };

        std::string GetFullPath() const;

        ShaderNode(const char* name, const u32 offset, const Size size, const ShaderNodeType type) : name(name),
            offset(offset), size(size), type(type)
        {
            children.reserve(MAX_CHILDREN);
        }

        bool GetScope(std::string& scopeOut) const;
        ShaderNode* GetChild(Size index);
        ShaderNode* GetChildByName(const std::string& name);
        ShaderNode* GetChildByIndex(Size index);
        ShaderNode* GetChildByHandle(ShaderNodeHandle handle);
        ShaderNode* AppendChild(const char* name, const u32 offset, const Size size, const ShaderNodeType type);

        void Print();
        static void Print(ShaderNode* node, u32 indent);

    private:
        std::vector<SPtr<ShaderNode>> children;
    };
}
