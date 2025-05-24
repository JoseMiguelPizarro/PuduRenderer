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
        cursor.m_descriptorType = node->binding.type;
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

    ShaderNode* ShaderCursor::GetNode() const
    {
        return m_layout;
    }

    bool ShaderCursor::IsValid()
    {
        return m_isValid;
    }

    void ShaderCursor::Write(const SPtr<Texture>& texture)
    {
        if (texture == nullptr)
        {
            LOG_WARNING("Trying to set null texture Write for {}", m_layout->name);
            return;
        }
        if (!texture->IsAllocated())
        {
            LOG_WARNING(
                "Trying to set texture [{}] not yet allocated Write for {} Please call Texture::Create() before assigning it to a shader",
                texture->name, m_layout->name);
            return;
        }

        VkImageLayout layout = (m_descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
            m_descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = texture->vkImageViewHandle;
        imageInfo.sampler = texture->Sampler.vkHandle;
        imageInfo.imageLayout =  layout;

        VkWriteDescriptorSet imageWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        imageWrite.descriptorCount = 1;
        imageWrite.dstBinding = m_bindingIndex;
        imageWrite.dstSet = m_target->GetDescriptorSet(m_setIndex);
        imageWrite.pImageInfo = &imageInfo;
        imageWrite.descriptorType = m_descriptorType;

        //If the texture comes from a render target or has been written on a compute shader, we'd might need to transition its layout depending on the intended usage
        //So far there are two possible usages for images on descriptor, either being read on a shader or used as a RW texture on a compute shader
        if (layout != texture->GetImageLayout())
        {
            //Doing the transition here might not be the best solution. TODO: Consult with Tim or someone who can know more about sync
            auto cmd = m_target->graphics->BeginSingleTimeCommands();
            VkImageSubresourceRange range{};
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //TODO: GET THE RIGHT ASPECT MASK
            range.baseMipLevel = 0;
            range.levelCount = texture->mipLevels;
            range.baseArrayLayer = 0;
            range.layerCount = texture->layers;

            cmd.TransitionTextureLayout(texture.get(), layout, &range);
            m_target->graphics->EndSingleTimeCommands(cmd);
        }

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
        bufferWrite.dstSet = m_target->GetDescriptorSet(m_setIndex);
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
