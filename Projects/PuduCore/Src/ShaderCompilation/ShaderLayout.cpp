#include "ShaderCompilation/ShaderLayout.h"

#include "Logger.h"

namespace Pudu
{
#pragma region ShaderNode
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

    void ShaderNode::AppendChild(const ShaderNode& child)
    {
        children.push_back(child);
    }

    ShaderNode* ShaderNode::AppendChild(const char* name, const u32 offset, const Size size, const ShaderNodeType type)
    {
        const auto shaderNode = ShaderNode(name, offset, size, type);

        children.push_back(shaderNode);

        return &children.back();
    }


#pragma endregion ShaderNode
}
