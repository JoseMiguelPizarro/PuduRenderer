//
// Created by jpizarro on 3/11/2025.
//

#include "ShaderCursor/ShaderCursor.h"


namespace Pudu
{
    ShaderCursor ShaderCursor::Field(const char* name) const
    {
        auto node = m_layout->GetChildByName(name);

        auto cursor =  ShaderCursor(node);

        cursor.m_bindingIndex = node->bindingIndex;
        cursor.m_setIndex = node->setIndex;
    }

    ShaderCursor ShaderCursor::Field(u32 index) const
    {
        auto node = m_layout->GetChildByIndex(index);

        return ShaderCursor(node);
    }

    ShaderCursor ShaderCursor::Element(u32 index)
    {
        throw std::exception("ShaderCursor::Element Not Implemented");
    }
}
