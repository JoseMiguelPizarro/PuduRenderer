//
// Created by jpizarro on 3/11/2025.
//

#include "ShaderCursor/ShaderCursor.h"

#include "PuduGraphics.h"
#include "Texture.h"


namespace Pudu
{
    void ShaderCursor::SetupCursor(ShaderCursor& cursor, ShaderNode* node)
    {
        cursor.m_layout = node;
        cursor.m_byteOffset = node->offset;
        cursor.m_setIndex = node->setIndex;
        cursor.m_bindingIndex = node->bindingIndex;
        cursor.m_byteSize = node->size;
    }

    ShaderCursor ShaderCursor::Field(const char* name) const
    {
        std::stringstream ss(name);

        std::string token;

        ShaderNode* node = this->m_layout;

        while (std::getline(ss, token, ELEMENT_PATH_DELIMITER))
        {
            if (node == nullptr)
            {
                auto cursor = ShaderCursor(m_layout, m_target);
                cursor.m_isValid = false;
                return cursor;
            }
            node = node->GetChildByName(token);
        }

        auto cursor = ShaderCursor(node, m_target);

        SetupCursor(cursor, node);

        cursor.m_isValid = true;
        return cursor;
    }

    ShaderCursor ShaderCursor::Field(u32 index) const
    {
        const auto node = m_layout->GetChildByIndex(index);

        if (node == nullptr)
        {
            auto cursor = ShaderCursor(m_layout, m_target);
            cursor.m_isValid = false;
            return cursor;
        }

        auto cursor = ShaderCursor(node, m_target);
        SetupCursor(cursor, node);
        cursor.m_isValid = true;

        return cursor;
    }

    ShaderCursor ShaderCursor::Element(u32 index) const
    {
        ShaderCursor cursor = *this;
        cursor.m_bindingArrayIndex = index;

        return cursor;
    }


    void ShaderCursor::Write(const SPtr<Texture>& texture)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = texture->vkImageViewHandle;
        imageInfo.sampler = texture->Sampler.vkHandle;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet imageWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        imageWrite.descriptorCount = 1;
        imageWrite.dstBinding = m_bindingIndex;
        imageWrite.dstSet = m_target->descriptorSets[m_setIndex];
        imageWrite.pImageInfo = &imageInfo;
        imageWrite.descriptorType = m_descriptorType;

        m_target->graphics->UpdateDescriptorSet(1, &imageWrite);
    }

    void ShaderCursor::Write(const SPtr<GraphicsBuffer>& buffer)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer->vkHandle;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet bufferWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        bufferWrite.descriptorCount = 1;
        bufferWrite.dstBinding = m_bindingIndex;
        bufferWrite.dstSet = m_target->descriptorSets[m_setIndex];
        bufferWrite.pBufferInfo = &bufferInfo;
        bufferWrite.descriptorType = m_descriptorType;

        m_target->graphics->UpdateDescriptorSet(1, &bufferWrite);
    }

    void ShaderCursor::Write(const SPtr<GraphicsBuffer>& buffer, const void* data, Size offset, Size size)
    {
        m_target->graphics->UploadBufferData(buffer.get(), data, size, offset);
    }

    void ShaderCursor::Write(f32 value)
    {

    }

    void ShaderCursor::Write(u32 value)
    {
    }

    void ShaderCursor::Write(vec2 value)
    {
    }

    void ShaderCursor::Write(vec3 value)
    {
    }

    void ShaderCursor::Write(vec4 value)
    {
    }

    void ShaderCursor::Write(mat2 value)
    {
    }

    void ShaderCursor::Write(mat3 value)
    {
    }

    void ShaderCursor::Write(mat4 value)
    {
    }
}
