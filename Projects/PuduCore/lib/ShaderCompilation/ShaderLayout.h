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
        CBuffer,
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

    struct ShaderNode
    {
        std::string name;
        u32 offset{};
        Size size{};
        Size elementCount;

        u32 bindingIndex;
        u32 setIndex;

        ShaderNodeType type;

        Size childCount = 0;
        DescriptorBinding binding;
        std::string scope;

        ShaderNode()
        {
            children.reserve(MAX_CHILDREN);
        };

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
        ShaderNode* AppendChild(const ShaderNode& child);
        ShaderNode* AppendChild(const char* name, const u32 offset, const Size size, const ShaderNodeType type);

        void Print();
        static void Print(ShaderNode* node, u32 indent);

    private:
        std::vector<ShaderNode> children;
    };
}
