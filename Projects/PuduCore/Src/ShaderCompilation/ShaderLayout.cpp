#include "ShaderCompilation/ShaderLayout.h"

#include <iostream>

#include "Logger.h"

namespace Pudu
{
#pragma region ShaderNode
    ShaderNode* ShaderNode::GetChild(Size index)
    {
        ASSERT(index < childCount, "ShaderNode: Child index out of bounds!");
        return &children[index];
    }

    ShaderNode* ShaderNode::GetChildByName(const std::string& name)
    {
        for (Size i = 0; i < children.size(); i++)
        {
            if (children[i].name == name)
            {
                return &children[i];
            }
        }

        LOG_WARNING("ShaderNode child not found with name {}", name);
        return nullptr;
    }

    ShaderNode* ShaderNode::GetChildByIndex(const Size index)
    {
        if (index >= children.size())
        {
            LOG_WARNING("ShaderNode child not found with index {}", index);
            return nullptr;
        }

        return &children[index];
    }

    ShaderNode* ShaderNode::GetChildByHandle(ShaderNodeHandle handle)
    {
        return GetChildByIndex(handle.index);
    }

    ShaderNode* ShaderNode::AppendChild(const ShaderNode& child)
    {
        ASSERT(children.size() < MAX_CHILDREN, "Reached maximum number of children for shader node {}!", this->name);

        children.push_back(child);

        return &children.back();
    }

    ShaderNode* ShaderNode::AppendChild(const char* name, const u32 offset, const Size size,
                                        const ShaderNodeType type)
    {
        ASSERT(children.size() < MAX_CHILDREN, "Reached maximum number of children for shader node {}!", this->name);

        children.resize(children.size() + 1);

        const auto shaderNode = &children[children.size() - 1];
        shaderNode->name = name;
        shaderNode->offset = offset;
        shaderNode->size = size;
        shaderNode->type = type;

        return shaderNode;
    }

    void ShaderNode::Print()
    {
        Print(this, 0);
    }

    void ShaderNode::Print(ShaderNode* node, u32 indent)
    {
        if (node == nullptr)
            return;

        LOG_I(indent, "{}", node->name);

        for (Size i = 0; i < node->children.size(); i++)
        {
            Print(&node->children[i], indent + 1);
        }
    }
#pragma endregion ShaderNode
}
