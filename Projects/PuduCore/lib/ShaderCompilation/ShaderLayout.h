#pragma once
#include <string>

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
    };

    struct ShaderNode
    {
        std::string name;
        u32 offset{};
        Size size{};
        Size elementCount;

        u32 bindingIndex;
        u32 setIndex;

        ShaderNodeType type;

        ShaderNode() = default;

        ShaderNode(const char* name, const u32 offset, const Size size, const ShaderNodeType type) : name(name),
            offset(offset), size(size), type(type)
        {
        }

        ShaderNode* GetChildByName(const std::string& name);
        ShaderNode* GetChildByIndex(Size index);
        void AppendChild(const ShaderNode& child);
        ShaderNode* AppendChild(const char* name, const u32 offset, const Size size, const ShaderNodeType type);

        void Print();
        static void Print(ShaderNode* node, u32 indent);

    private:
        std::vector<ShaderNode> children;
    };
}
