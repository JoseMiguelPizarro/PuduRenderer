//
// Created by jpizarro on 3/11/2025.
//

#pragma once
#include <slang/slang.h>

#include "GraphicsBuffer.h"
#include "Material.h"
#include "PuduCore.h"
#include "ShaderCompilation/ShaderLayout.h"

namespace Pudu
{
    class PuduGraphics;

    class ShaderCursor
    {
    public:
        ShaderCursor(ShaderNode* layout, const MaterialApplyPropertyGPUTarget* propertyTarget)
        {
            m_layout = layout;
            m_target = propertyTarget;
        }

        static void SetupCursor(ShaderCursor& cursor, ShaderNode* node);
        ShaderCursor Field(const char* name) const;
        ShaderCursor Field(u32 index) const;
        ShaderCursor Element(u32 index) const;

        bool IsValid();

        void Write(const SPtr<GraphicsBuffer>& buffer, const void* data, Size offset, Size size);
        void Write(const SPtr<Texture>& texture);
        void Write(f32 value);
        void Write(u32 value);
        void Write(vec2 value);
        void Write(vec3 value);
        void Write(vec4 value);
        void Write(mat2 value);
        void Write(mat3 value);
        void Write(mat4 value);
        void Write(const SPtr<GraphicsBuffer>& buffer);

    private:
        SPtr<GraphicsBuffer> m_buffer;
        std::byte* m_bufferData;
        Size m_byteOffset;
        Size m_byteSize;
        u32 m_setIndex;
        u32 m_bindingIndex;
        u32 m_bindingArrayIndex;
        VkDescriptorType m_descriptorType;
        const MaterialApplyPropertyGPUTarget* m_target;
        bool m_isValid;

        ShaderNode* m_layout;

        constexpr char ELEMENT_PATH_DELIMITER = '.';
    };
}
