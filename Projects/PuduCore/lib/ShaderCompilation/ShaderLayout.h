#pragma once
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

#include "DescriptorSetLayoutInfo.h"
#include "../PuduCore.h"

namespace Pudu
{
    //🐞 DON'T FORGET TO MODIFY SHADER_NODE_TYPE_NAMES WHEN MODIFYING THIS
    enum ShaderNodeType
    {
        Root,
        //THIS IS RESERVED FOR CBUFFER USUALLY AUTOMATICALLY ALLOCATED
        CBuffer,
        Buffer,
        Array,
        Resource,
        Uniform,
        PushConstant,
        ParameterBlock,
        Struct,
    };


    static std::map<ShaderNodeType, const char*> SHADER_NODE_TYPE_NAMES = {
        {Root, "Root"},
        {CBuffer, "CBuffer"},
        {Buffer, "Buffer"},
        {Array, "Array"},
        {Resource, "Resource"},
        {Uniform, "Uniform"},
        {PushConstant, "PushConstant"},
        {ParameterBlock, "ParameterBlock"},
        {Struct, "Struct"}
    };

    struct ShaderNodeHandle
    {
        Size index;
    };

    constexpr Size MAX_CHILDREN = 64;
    static const char* ROOT_NAME = "Root";

    struct ShaderNode
    {
        class Shape
        {
        public: enum Flags
            {
                RESOURCE_BASE_SHAPE_MASK = 0x0F,

                RESOURCE_NONE = 0x00,

                TEXTURE_1D = 0x01,
                TEXTURE_2D = 0x02,
                TEXTURE_3D = 0x03,
                TEXTURE_CUBE = 0x04,
                TEXTURE_BUFFER = 0x05,

                STRUCTURED_BUFFER = 0x06,
                BYTE_ADDRESS_BUFFER = 0x07,
                RESOURCE_UNKNOWN = 0x08,
                ACCELERATION_STRUCTURE = 0x09,
                TEXTURE_SUBPASS = 0x0A,

                RESOURCE_EXT_SHAPE_MASK = 0xF0,

                TEXTURE_FEEDBACK_FLAG = 0x10,
                TEXTURE_SHADOW_FLAG = 0x20,
                TEXTURE_ARRAY_FLAG = 0x40,
                TEXTURE_MULTISAMPLE_FLAG = 0x80,

                TEXTURE_1D_ARRAY = TEXTURE_1D | TEXTURE_ARRAY_FLAG,
                TEXTURE_2D_ARRAY = TEXTURE_2D | TEXTURE_ARRAY_FLAG,
                TEXTURE_CUBE_ARRAY = TEXTURE_CUBE | TEXTURE_ARRAY_FLAG,

                TEXTURE_2D_MULTISAMPLE = TEXTURE_2D | TEXTURE_MULTISAMPLE_FLAG,
                TEXTURE_2D_MULTISAMPLE_ARRAY =
                TEXTURE_2D | TEXTURE_MULTISAMPLE_FLAG | TEXTURE_ARRAY_FLAG,
                TEXTURE_SUBPASS_MULTISAMPLE = TEXTURE_SUBPASS | TEXTURE_MULTISAMPLE_FLAG,
            };
        };

        std::string name;
        u32 offset = 0;
        Size size = 0;
        Size elementCount = 0;

        u32 bindingIndex = 0;
        u32 setIndex = 0;
        u32 setId = 0;

        ShaderNodeType type;

        Size childCount = 0;
        DescriptorBinding binding;
        std::string scope;
        Shape::Flags shape = Shape::RESOURCE_NONE;

        //Pointer to the final container this shader node will part of. ie CBuffer->Struct->floatValue for floatValue its container is CBuffer while its parent is Struct
        ShaderNode* parentContainer = nullptr;
        ShaderNode* parent = nullptr;


        ShaderNode()
        {
            children.reserve(MAX_CHILDREN);
        };

        std::string GetFullPath() const;

        ShaderNode(const char* name, const u32 offset, const Size size, const ShaderNodeType type) : name(name),
            offset(offset), size(size), type(type)
        {
            children.reserve(MAX_CHILDREN);
        }

        bool GetScope(std::string& scopeOut) const;
        ShaderNode* GetChild(Size index);
        ShaderNode* GetChildByName(const std::string& name);
        ShaderNode* GetChildByIndex(Size index);
        ShaderNode* GetChildByHandle(ShaderNodeHandle handle);
        ShaderNode* AppendChild(const char* name, const u32 offset, const Size size, const ShaderNodeType type);

        void Print();
        static void Print(ShaderNode* node, u32 indent);

    private:
        std::vector<SPtr<ShaderNode>> children;
    };
}
