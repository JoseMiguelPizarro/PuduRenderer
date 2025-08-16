#pragma once
#include <array>
#include <vulkan/vulkan_core.h>

namespace Pudu
{
    enum class ChannelFormat
    {
        UNDEFINED = 0,
        R4G4_UNORM_PACK8 = 1,
        R4G4B4A4_UNORM_PACK16 = 2,
        B4G4R4A4_UNORM_PACK16 = 3,
        R5G6B5_UNORM_PACK16 = 4,
        B5G6R5_UNORM_PACK16 = 5,
        R5G5B5A1_UNORM_PACK16 = 6,
        B5G5R5A1_UNORM_PACK16 = 7,
        A1R5G5B5_UNORM_PACK16 = 8,
        R8_UNORM = 9,
        R8_SNORM = 10,
        R8_USCALED = 11,
        R8_SSCALED = 12,
        R8_UINT = 13,
        R8_SINT = 14,
        R8_SRGB = 15,
        R8G8_UNORM = 16,
        R8G8_SNORM = 17,
        R8G8_USCALED = 18,
        R8G8_SSCALED = 19,
        R8G8_UINT = 20,
        R8G8_SINT = 21,
        R8G8_SRGB = 22,
        R8G8B8_UNORM = 23,
        R8G8B8_SNORM = 24,
        R8G8B8_USCALED = 25,
        R8G8B8_SSCALED = 26,
        R8G8B8_UINT = 27,
        R8G8B8_SINT = 28,
        R8G8B8_SRGB = 29,
        B8G8R8_UNORM = 30,
        B8G8R8_SNORM = 31,
        B8G8R8_USCALED = 32,
        B8G8R8_SSCALED = 33,
        B8G8R8_UINT = 34,
        B8G8R8_SINT = 35,
        B8G8R8_SRGB = 36,
        R8G8B8A8_UNORM = 37,
        R8G8B8A8_SNORM = 38,
        R8G8B8A8_USCALED = 39,
        R8G8B8A8_SSCALED = 40,
        R8G8B8A8_UINT = 41,
        R8G8B8A8_SINT = 42,
        R8G8B8A8_SRGB = 43,
        B8G8R8A8_UNORM = 44,
        B8G8R8A8_SNORM = 45,
        B8G8R8A8_USCALED = 46,
        B8G8R8A8_SSCALED = 47,
        B8G8R8A8_UINT = 48,
        B8G8R8A8_SINT = 49,
        B8G8R8A8_SRGB = 50,
        A8B8G8R8_UNORM_PACK32 = 51,
        A8B8G8R8_SNORM_PACK32 = 52,
        A8B8G8R8_USCALED_PACK32 = 53,
        A8B8G8R8_SSCALED_PACK32 = 54,
        A8B8G8R8_UINT_PACK32 = 55,
        A8B8G8R8_SINT_PACK32 = 56,
        A8B8G8R8_SRGB_PACK32 = 57,
        A2R10G10B10_UNORM_PACK32 = 58,
        A2R10G10B10_SNORM_PACK32 = 59,
        A2R10G10B10_USCALED_PACK32 = 60,
        A2R10G10B10_SSCALED_PACK32 = 61,
        A2R10G10B10_UINT_PACK32 = 62,
        A2R10G10B10_SINT_PACK32 = 63,
        A2B10G10R10_UNORM_PACK32 = 64,
        A2B10G10R10_SNORM_PACK32 = 65,
        A2B10G10R10_USCALED_PACK32 = 66,
        A2B10G10R10_SSCALED_PACK32 = 67,
        A2B10G10R10_UINT_PACK32 = 68,
        A2B10G10R10_SINT_PACK32 = 69,
        R16_UNORM = 70,
        R16_SNORM = 71,
        R16_USCALED = 72,
        R16_SSCALED = 73,
        R16_UINT = 74,
        R16_SINT = 75,
        R16_SFLOAT = 76,
        R16G16_UNORM = 77,
        R16G16_SNORM = 78,
        R16G16_USCALED = 79,
        R16G16_SSCALED = 80,
        R16G16_UINT = 81,
        R16G16_SINT = 82,
        R16G16_SFLOAT = 83,
        R16G16B16_UNORM = 84,
        R16G16B16_SNORM = 85,
        R16G16B16_USCALED = 86,
        R16G16B16_SSCALED = 87,
        R16G16B16_UINT = 88,
        R16G16B16_SINT = 89,
        R16G16B16_SFLOAT = 90,
        R16G16B16A16_UNORM = 91,
        R16G16B16A16_SNORM = 92,
        R16G16B16A16_USCALED = 93,
        R16G16B16A16_SSCALED = 94,
        R16G16B16A16_UINT = 95,
        R16G16B16A16_SINT = 96,
        R16G16B16A16_SFLOAT = 97,
        R32_UINT = 98,
        R32_SINT = 99,
        R32_SFLOAT = 100,
        R32G32_UINT = 101,
        R32G32_SINT = 102,
        R32G32_SFLOAT = 103,
        R32G32B32_UINT = 104,
        R32G32B32_SINT = 105,
        R32G32B32_SFLOAT = 106,
        R32G32B32A32_UINT = 107,
        R32G32B32A32_SINT = 108,
        R32G32B32A32_SFLOAT = 109,
        R64_UINT = 110,
        R64_SINT = 111,
        R64_SFLOAT = 112,
        R64G64_UINT = 113,
        R64G64_SINT = 114,
        R64G64_SFLOAT = 115,
        R64G64B64_UINT = 116,
        R64G64B64_SINT = 117,
        R64G64B64_SFLOAT = 118,
        R64G64B64A64_UINT = 119,
        R64G64B64A64_SINT = 120,
        R64G64B64A64_SFLOAT = 121,
    };

    static constexpr std::array<Size, 122> ChannelFormatSize = {
        0, // UNDEFINED
        1, // R4G4_UNORM_PACK8
        2, // R4G4B4A4_UNORM_PACK16
        2, // B4G4R4A4_UNORM_PACK16
        2, // R5G6B5_UNORM_PACK16
        2, // B5G6R5_UNORM_PACK16
        2, // R5G5B5A1_UNORM_PACK16
        2, // B5G5R5A1_UNORM_PACK16
        2, // A1R5G5B5_UNORM_PACK16
        1, // R8_UNORM
        1, // R8_SNORM
        1, // R8_USCALED
        1, // R8_SSCALED
        1, // R8_UINT
        1, // R8_SINT
        1, // R8_SRGB
        2, // R8G8_UNORM
        2, // R8G8_SNORM
        2, // R8G8_USCALED
        2, // R8G8_SSCALED
        2, // R8G8_UINT
        2, // R8G8_SINT
        2, // R8G8_SRGB
        3, // R8G8B8_UNORM
        3, // R8G8B8_SNORM
        3, // R8G8B8_USCALED
        3, // R8G8B8_SSCALED
        3, // R8G8B8_UINT
        3, // R8G8B8_SINT
        3, // R8G8B8_SRGB
        3, // B8G8R8_UNORM
        3, // B8G8R8_SNORM
        3, // B8G8R8_USCALED
        3, // B8G8R8_SSCALED
        3, // B8G8R8_UINT
        3, // B8G8R8_SINT
        3, // B8G8R8_SRGB
        4, // R8G8B8A8_UNORM
        4, // R8G8B8A8_SNORM
        4, // R8G8B8A8_USCALED
        4, // R8G8B8A8_SSCALED
        4, // R8G8B8A8_UINT
        4, // R8G8B8A8_SINT
        4, // R8G8B8A8_SRGB
        4, // B8G8R8A8_UNORM
        4, // B8G8R8A8_SNORM
        4, // B8G8R8A8_USCALED
        4, // B8G8R8A8_SSCALED
        4, // B8G8R8A8_UINT
        4, // B8G8R8A8_SINT
        4, // B8G8R8A8_SRGB
        4, // A8B8G8R8_UNORM_PACK32
        4, // A8B8G8R8_SNORM_PACK32
        4, // A8B8G8R8_USCALED_PACK32
        4, // A8B8G8R8_SSCALED_PACK32
        4, // A8B8G8R8_UINT_PACK32
        4, // A8B8G8R8_SINT_PACK32
        4, // A8B8G8R8_SRGB_PACK32
        4, // A2R10G10B10_UNORM_PACK32
        4, // A2R10G10B10_SNORM_PACK32
        4, // A2R10G10B10_USCALED_PACK32
        4, // A2R10G10B10_SSCALED_PACK32
        4, // A2R10G10B10_UINT_PACK32
        4, // A2R10G10B10_SINT_PACK32
        4, // A2B10G10R10_UNORM_PACK32
        4, // A2B10G10R10_SNORM_PACK32
        4, // A2B10G10R10_USCALED_PACK32
        4, // A2B10G10R10_SSCALED_PACK32
        4, // A2B10G10R10_UINT_PACK32
        4, // A2B10G10R10_SINT_PACK32
        2, // R16_UNORM
        2, // R16_SNORM
        2, // R16_USCALED
        2, // R16_SSCALED
        2, // R16_UINT
        2, // R16_SINT
        2, // R16_SFLOAT
        4, // R16G16_UNORM
        4, // R16G16_SNORM
        4, // R16G16_USCALED
        4, // R16G16_SSCALED
        4, // R16G16_UINT
        4, // R16G16_SINT
        4, // R16G16_SFLOAT
        6, // R16G16B16_UNORM
        6, // R16G16B16_SNORM
        6, // R16G16B16_USCALED
        6, // R16G16B16_SSCALED
        6, // R16G16B16_UINT
        6, // R16G16B16_SINT
        6, // R16G16B16_SFLOAT
        8, // R16G16B16A16_UNORM
        8, // R16G16B16A16_SNORM
        8, // R16G16B16A16_USCALED
        8, // R16G16B16A16_SSCALED
        8, // R16G16B16A16_UINT
        8, // R16G16B16A16_SINT
        8, // R16G16B16A16_SFLOAT
        4, // R32_UINT
        4, // R32_SINT
        4, // R32_SFLOAT
        8, // R32G32_UINT
        8, // R32G32_SINT
        8, // R32G32_SFLOAT
        12, // R32G32B32_UINT
        12, // R32G32B32_SINT
        12, // R32G32B32_SFLOAT
        16, // R32G32B32A32_UINT
        16, // R32G32B32A32_SINT
        16, // R32G32B32A32_SFLOAT
        8, // R64_UINT
        8, // R64_SINT
        8, // R64_SFLOAT
        16, // R64G64_UINT
        16, // R64G64_SINT
        16, // R64G64_SFLOAT
        24, // R64G64B64_UINT
        24, // R64G64B64_SINT
        24, // R64G64B64_SFLOAT
        32, // R64G64B64A64_UINT
        32, // R64G64B64A64_SINT
        32, // R64G64B64A64_SFLOAT
    };

    static constexpr Size GetChannelFormatSize(ChannelFormat format)
    {
        return ChannelFormatSize[static_cast<u32>(format)];
    }

    static constexpr VkFormat ToVk(ChannelFormat format)
    {
        return static_cast<VkFormat>(format);
    }

    enum LoadOperation
    {
        DontCare,
        Load,
        Clear,
    }; // enum Enum

    enum AttachmentAccessUsage
    {
        Read = 1,
        Write = 2,
        ReadAndWrite = 3,
        Sample = 4,
        CopySrc = 8,
        CopyDst = 16,
    };

    enum CullMode
    {
        Front,
        Back,
        Both,
        None
    };

    enum class BlendingMode
    {
        Opaque,
        AlphaBlend,
        AdditiveBlend,
        SubtractiveBlend,
        MultiplyBlend,
        CustomBlend
    };

    enum class PresentMode
    {
        IMMEDIATE = 0,
        MAILBOX = 1,
        FIFO = 2,
        FIFO_RELAXED = 3,
    };

    inline VkPresentModeKHR ToVk(PresentMode mode)
    {
        return static_cast<VkPresentModeKHR>(mode);
    }

    static VkCullModeFlagBits ToVk(CullMode cullMode)
    {
        switch (cullMode)
        {
        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Both:
            return VK_CULL_MODE_FRONT_AND_BACK;
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        default:
            return VK_CULL_MODE_BACK_BIT;
            break;
        }
    }

    static VkAttachmentLoadOp ToVk(LoadOperation op)
    {
        switch (op)
        {
        case Pudu::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        case Pudu::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case Pudu::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        default:
            break;
        }

        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    static VkAttachmentStoreOp ToVk(AttachmentAccessUsage usage)
    {
        switch (usage)
        {
        case Pudu::ReadAndWrite:
            return VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case Pudu::Write:
            return VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case Pudu::Read:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case Pudu::Sample:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case CopyDst:
            return VK_ATTACHMENT_STORE_OP_STORE;
        default:
            break;
        }

        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}
