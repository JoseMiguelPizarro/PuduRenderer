#include "ShaderCursor.h"
#include "PuduGraphics.h"

namespace Pudu {
    ShaderCursor ShaderCursor::field(const char* name)
    {
        return field(m_typeLayout->findFieldIndexByName(name));
    }
    ShaderCursor ShaderCursor::field(int index)
    {
        slang::VariableLayoutReflection* field = m_typeLayout->getFieldByIndex(index);

        ShaderCursor result = *this;
        result.m_typeLayout = field->getTypeLayout();
        result.m_offset.byteOffset+= field->getOffset();
        result.m_offset.bindingRangeIndex += m_typeLayout->getFieldBindingRangeOffset(index);

        return result;
    }
    ShaderCursor ShaderCursor::element(int index)
    {
        slang::TypeLayoutReflection* elementTypeLayout =
            m_typeLayout->getElementTypeLayout();

        ShaderCursor result = *this;
        result.m_typeLayout = elementTypeLayout;
        result.m_offset.byteOffset += index * elementTypeLayout->getStride();

        result.m_offset.arrayIndexInBindingRange *= m_typeLayout->getElementCount();
        result.m_offset.arrayIndexInBindingRange += index;


        return result;
    }
    void ShaderCursor::Write(Texture* texture)
    {
        m_object->Write(m_offset, texture);
    }

    void ShaderCursor::Write(const void* data, size_t size)
    {
        m_object->Write(m_offset, data, size);
    }
}