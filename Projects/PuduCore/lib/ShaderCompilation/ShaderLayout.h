#pragma once
#include <string>

#include "../PuduCore.h"

namespace Pudu
{
    struct ShaderNode
    {
        std::string name;
        u32 offset;
        Size size;

        u32 bindingIndex;
        u32 setIndex;

        ShaderNode() = default;
        ShaderNode(const char* name, const u32 offset, const Size size) : name(name), offset(offset), size(size) {}

        ShaderNode* GetChildByName(const std::string& name);
        ShaderNode* GetChildByIndex(const Size index);
        void AppendChild(const ShaderNode& child);


    private:
        std::vector<ShaderNode> children;
    };

    class ShaderLayout
    {
    public:
        ShaderNode* GetRoot() { return &m_root; };
        ShaderLayout();

    private:
        ShaderNode m_root;
    };
}
