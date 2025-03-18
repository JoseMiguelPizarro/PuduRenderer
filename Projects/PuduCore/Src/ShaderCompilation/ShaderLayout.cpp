#include "ShaderCompilation/ShaderLayout.h"

#include <iostream>

#include "Logger.h"

namespace Pudu
{
#pragma region ShaderNode
    bool ShaderNode::GetScope(std::string& scopeOut) const
    {
        if (scope.empty())
            return false;

        scopeOut = scope;
        return true;
    }

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

        if (this->type == ShaderNodeType::CBuffer)
        {
            shaderNode->parentContainer = this;
        }
        else
        {
            shaderNode->parentContainer = this->parentContainer;
        }

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

        switch (node->type)
        {
        case Root:
            LOG_I(indent * 2, "Root");
            break;
        case CBuffer:
            {
                LOG_I(indent * 2, "{} Type: {} Size: {} [set:{} index:{}]",
                      node->name,
                      SHADER_NODE_TYPE_NAMES.at(node->type),
                      node->size,
                      node->setIndex,
                      node->bindingIndex);
            }
            break;
        case Array:
            {
                LOG_I(indent * 2, "{} Type: {} count: {} [set:{} index:{}]",
                      node->name,
                      SHADER_NODE_TYPE_NAMES.at(node->type),
                      node->elementCount,
                      node->setIndex,
                      node->bindingIndex);
            }
            break;
        case Resource:
            {
                LOG_I(indent * 2, "{} Type: {} [set:{} index:{}]",
                      node->name,
                      SHADER_NODE_TYPE_NAMES.at(node->type),
                      node->setIndex,
                      node->bindingIndex);
            }
            break;
        case Uniform:
            {
                LOG_I(indent * 2, "{} Type: {} (size: {} offset: {})",
                      node->name,
                      SHADER_NODE_TYPE_NAMES.at(node->type),
                      node->size,
                      node->offset);
            }
            break;
        case PushConstant:
            LOG_I(indent * 2, "{} Type: {} Size: {}",
                  node->name,
                  SHADER_NODE_TYPE_NAMES.at(node->type),
                  node->size);
            break;
        case ParameterBlock:
            LOG_I(indent * 2, "{} Type: {} [set:{}]", node->name, SHADER_NODE_TYPE_NAMES.at(node->type),
                  node->setIndex);
            break;
        case Struct:
            LOG_I(indent * 2,
                  "{} Type: {} [set:{} index:{}]",
                  node->name, SHADER_NODE_TYPE_NAMES.at(node->type),
                  node->setIndex, node->bindingIndex);
            break;
        }

        for (Size i = 0; i < node->children.size(); i++)
        {
            Print(&node->children[i], indent + 1);
        }
    }
#pragma endregion ShaderNode
}
