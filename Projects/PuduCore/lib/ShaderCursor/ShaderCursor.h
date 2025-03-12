//
// Created by jpizarro on 3/11/2025.
//

#pragma once
#include <slang/slang.h>

#include "GraphicsBuffer.h"
#include "PuduCore.h"

namespace Pudu
{
    class ShaderCursor
    {
    public:
        ShaderCursor Field(const char* name);
        ShaderCursor Field(u32 index);
        ShaderCursor Element(u32 index);

        void Write(const void* data, size_t size);
        void Write(SPtr<Texture> texture);
        void Write(f32 value);
        void Write(u32 value);
        void Write(vec2 value);
        void Write(vec3 value);
        void Write(vec4 value);
        void Write(mat2 value);
        void Write(mat3 value);
        void Write(mat4 value);

    private:
    private:
        SPtr<GraphicsBuffer> m_buffer;
        std::byte* m_bufferData;
        size m_byteOffset;
        VkDescriptorSet m_descriptorSet;
        u32 m_bindingIndex;
        u32 m_bindingArrayIndex;

        slang::TypeLayoutReflection m_typeLayout;
    };
}
