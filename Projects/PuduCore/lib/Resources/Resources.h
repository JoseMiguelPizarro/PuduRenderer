#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "DescriptorSetLayoutCollection.h"
#include "PuduCore.h"
#include "DescriptorSetLayoutInfo.h"
#include "GPUResource.h"
#include "PuduConstants.h"

namespace Pudu
{
    class RenderPass;
    class RenderTexture;
    class Texture;

#pragma region Handles
    typedef u32 FrameGraphHandle;
    typedef u32 FrameGraphNodeHandle;
    typedef u32 ResourceHandle;

#pragma endregion

    namespace QueueType
    {
        enum Enum
        {
            Graphics,
            Compute,
            CopyTransfer,
            Count
        };

        enum Mask
        {
            Graphics_mask = 1 << 0,
            Compute_mask = 1 << 1,
            CopyTransfer_mask = 1 << 2,
            Count_mask = 1 << 3
        };

        static const char *s_value_names[] = {
            "Graphics", "Compute", "CopyTransfer", "Count"};

        static const char *ToString(Enum e)
        {
            return ((u32)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
        }
    }

#pragma region RenderPass

    namespace ColorMask
    {
        enum Enum
        {
            Red = 1 << 0,
            Green = 1 << 1,
            Blue = 1 << 2,
            Alpha = 1 << 3,
            All = Red | Green | Blue | Alpha,
        };

        static const char *s_value_names[] = {
            "Red", "Green", "Blue", "Alpha", "All", "Count"};

        static const char *ToString(const Enum e)
        {
            return (s_value_names[static_cast<int>(e)]);
        }
    }

    class PuduGraphics;

#pragma endregion

    namespace FillMode
    {
        enum Enum
        {
            Wireframe,
            Solid,
            Point,
            Count
        };

        enum Mask
        {
            Wireframe_mask = 1 << 0,
            Solid_mask = 1 << 1,
            Point_mask = 1 << 2,
            Count_mask = 1 << 3
        };

        static const char *s_value_names[] = {
            "Wireframe", "Solid", "Point", "Count"};

        static const char *ToString(Enum e)
        {
            return (static_cast<u32>(e) < Enum::Count ? s_value_names[static_cast<int>(e)] : "unsupported");
        }
    } // namespace FillMode

    struct StencilOperationState
    {
        VkStencilOp failOp = VK_STENCIL_OP_KEEP;
        VkStencilOp passOp = VK_STENCIL_OP_KEEP;
        VkStencilOp depthFail = VK_STENCIL_OP_KEEP;
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
        u32 compareMask = 0xff;
        u32 writeMask = 0xff;
        u32 reference = 0xff;

        static VkStencilOpState GetVkOpState(StencilOperationState state)
        {
            VkStencilOpState vkstate{};
            vkstate.compareOp = state.compareOp;
            vkstate.compareMask = state.compareMask;
            vkstate.passOp = state.passOp;
            vkstate.failOp = state.failOp;
            vkstate.reference = state.reference;
            vkstate.writeMask = state.writeMask;
            vkstate.depthFailOp = state.depthFail;

            return vkstate;
        }
    }; // struct StencilOperationState

    struct DepthStencilCreation
    {
        StencilOperationState front;
        StencilOperationState back;
        VkCompareOp depthComparison = VK_COMPARE_OP_ALWAYS;

        u8 isDepthEnabled : 1;
        u8 isDepthWriteEnable : 1;
        u8 isStencilEnabled : 1;
        u8 pad : 5;

        // Default constructor
        DepthStencilCreation() : isDepthEnabled(0), isDepthWriteEnable(0), isStencilEnabled(0)
        {
        }

        DepthStencilCreation &SetDepth(bool write, VkCompareOp comparisonTest);

        VkStencilOpState GetVkFront() const { return StencilOperationState::GetVkOpState(front); }
        VkStencilOpState GetVkBack() const { return StencilOperationState::GetVkOpState(back); }
    }; // struct DepthStencilCreation

    struct BlendState
    {
        VkBlendFactor sourceColorFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor destinationColorFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp colorBlendOperation = VK_BLEND_OP_ADD;

        VkBlendFactor sourceAlphaFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor destinationAlphaFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp alphaBlendOperation = VK_BLEND_OP_ADD;

        ColorMask::Enum colorMask = ColorMask::All;

        u8 blendEnabled : 1;
        u8 pad : 6;

        BlendState() : blendEnabled(0)
        {
        }

        BlendState &SetColorBlending(VkBlendFactor sourceColor, VkBlendFactor destinationColor,
                                     VkBlendOp colorOperation);
        BlendState &SetAlphaBlending(VkBlendFactor sourceAlpha, VkBlendFactor destinationAlpha,
                                     VkBlendOp alphaOperation);
        BlendState &SetColorWriteMask(ColorMask::Enum value);
    }; // struct BlendState

    struct BlendStateCreation
    {
        BlendState blendStates[K_MAX_IMAGE_OUTPUTS];
        u32 activeStatesCount = 0;

        BlendState &AddBlendState();
    }; // BlendStateCreation

    //
    //
    struct RasterizationCreation
    {
        VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
        VkFrontFace front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        FillMode::Enum fill = FillMode::Solid;
    }; // struct RasterizationCreation

    struct ComputePipeline
    {
        VkPipeline vkHandle;
        VkPipelineLayout vkPipelineLayoutHandle;

        VkPipelineBindPoint vkPipelineBindPoint;
        VkDescriptorSet vkDescriptorSet = VK_FALSE; // Just 1 for now, bindless
        const DescriptorSetLayout *descriptorSetLayouts[K_MAX_DESCRIPTOR_SET_LAYOUTS];
        std::vector<GPUResourceHandle<DescriptorSetLayout>> descriptorSetLayoutHandles;
        bool bindlessUpdated;
    };

    namespace TextureType
    {
        enum Enum
        {
            Texture1D,
            Texture2D,
            Texture3D,
            Texture_1D_Array,
            Texture_2D_Array,
            Texture_Cube,
            Texture_Cube_Array,
        };
    } // namespace TextureType

    static VkImageCreateFlags ToVkImageFlags(TextureType::Enum type)
    {
        VkImageCreateFlags flags = {};
        switch (type)
        {
        case TextureType::Enum::Texture1D:
            break;
        case TextureType::Enum::Texture2D:
            break;
        case TextureType::Enum::Texture3D:
            break;
        case TextureType::Enum::Texture_1D_Array:
            break;
        case TextureType::Enum::Texture_2D_Array:
            break;
        case TextureType::Enum::Texture_Cube:
            flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        case TextureType::Enum::Texture_Cube_Array:
            break;
        default:
            break;
        }
        return flags;
    }

    static VkImageType ToVkImageType(TextureType::Enum type)
    {
        switch (type)
        {
        case Pudu::TextureType::Texture1D:
            return VK_IMAGE_TYPE_1D;
        case Pudu::TextureType::Texture2D:
            return VK_IMAGE_TYPE_2D;
            break;
        case Pudu::TextureType::Texture3D:
            return VK_IMAGE_TYPE_3D;
            break;
        case Pudu::TextureType::Texture_1D_Array:
            return VK_IMAGE_TYPE_1D;
            break;
        case Pudu::TextureType::Texture_2D_Array:
            return VK_IMAGE_TYPE_2D;
            break;
        case Pudu::TextureType::Texture_Cube:
            return VK_IMAGE_TYPE_2D;
            break;
        case Pudu::TextureType::Texture_Cube_Array:
            return VK_IMAGE_TYPE_3D;
            break;
        default:
            return VK_IMAGE_TYPE_2D;
            break;
        }
    }

    static VkImageViewType ToVkImageViewType(TextureType::Enum type)
    {
        static VkImageViewType s_vk_data[] = {
            VK_IMAGE_VIEW_TYPE_1D,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_3D,
            VK_IMAGE_VIEW_TYPE_1D_ARRAY,
            VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            VK_IMAGE_VIEW_TYPE_CUBE,
            VK_IMAGE_VIEW_TYPE_CUBE_ARRAY};
        return s_vk_data[type];
    }

    namespace TextureFlags
    {
        enum Enum
        {
            Default = 1,
            RenderTarget = 2,
            UnorderedAccess = 4,
            Count = 8,
            Sample = 16,
        };

        enum Mask
        {
            DefaultMask = 1 << 0,
            RenderTargetMask = 1 << 1,
            ComputeMask = 1 << 2
        };

        static const char *s_value_names[] = {
            "Default", "RenderTarget", "Compute", "Count", "Sample"};

        static const char *ToString(Enum e)
        {
            return ((u32)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
        }
    } // namespace TextureFlags

    namespace TextureFormat
    {
        inline bool IsDepthStencil(VkFormat value)
        {
            return value == VK_FORMAT_D16_UNORM_S8_UINT || value == VK_FORMAT_D24_UNORM_S8_UINT || value == VK_FORMAT_D32_SFLOAT_S8_UINT;
        }

        inline bool IsDepthOnly(VkFormat value)
        {
            return value >= VK_FORMAT_D16_UNORM && value < VK_FORMAT_D32_SFLOAT;
        }

        inline bool IsStencilOnly(VkFormat value)
        {
            return value == VK_FORMAT_S8_UINT;
        }

        inline bool HasDepth(VkFormat value)
        {
            return (value >= VK_FORMAT_D16_UNORM && value < VK_FORMAT_S8_UINT) || (value >=
                                                                                       VK_FORMAT_D16_UNORM_S8_UINT &&
                                                                                   value <= VK_FORMAT_D32_SFLOAT_S8_UINT);
        }

        inline bool HasStencil(VkFormat value)
        {
            return value >= VK_FORMAT_S8_UINT && value <= VK_FORMAT_D32_SFLOAT_S8_UINT;
        }

        inline bool HasDepthOrStencil(VkFormat value)
        {
            return value >= VK_FORMAT_D16_UNORM && value <= VK_FORMAT_D32_SFLOAT_S8_UINT;
        }
    } // namespace TextureFormat

    struct SamplerCreationData
    {
        bool wrap = true;
        u32 maxLOD = 1;
        VkFilter filter = VK_FILTER_LINEAR;
        std::string name;
    };

    /// <summary>
    /// Struct passed to load texture functions.
    /// </summary>
    struct TextureLoadSettings
    {
        const char *name = nullptr;
        bool bindless;
        bool generateMipmaps = false;
        i32 mipLevels = -1;
        TextureType::Enum textureType = TextureType::Texture2D;
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        SamplerCreationData samplerData;
    };

    struct TextureCreationData
    {
        u16 width = 1;
        u16 height = 1;
        u16 depth = 1;
        u8 mipmaps = 1;
        u32 dataSize = -1;
        u32 layers = 1;

        TextureFlags::Enum flags = TextureFlags::Default;

        SamplerCreationData *samplerData;
        VkFormat format;
        TextureType::Enum textureType = TextureType::Texture2D;
        bool bindless;
        void *pixels = nullptr;
        void *sourceData = nullptr;
        // ptr to source data, raw texture data if it exists (ie. raw loaded .ktx file) TODO: Remove, this is kinda ugly
        const char *name = nullptr;
        bool allocate = true;
        bool exposeMipViews = false;
        bool generateMipmaps = false;
    };

    struct Framebuffer : GPUResource<Framebuffer>
    {
        GPUResourceType Type() override { return GPUResourceType::Framebuffer; }
        VkFramebuffer vkHandle;

        GPUResourceHandle<RenderPass> renderPassHandle;

        u16 width = 0;
        u16 height = 0;

        float scaleX = 1.f;
        float scaleY = 1.f;

        GPUResourceHandle<Texture> colorAttachmentHandles[K_MAX_IMAGE_OUTPUTS];
        GPUResourceHandle<Texture> depthStencilAttachmentHandle;
        u32 numColorAttachments;

        u8 resize = 0;

        const char *name = nullptr;
    };

    /// <summary>
    /// Represents the data for the all shaders creation in a given pipeline
    /// </summary>
    struct ShaderState : GPUResource<ShaderState>
    {
        GPUResourceType Type() override { return GPUResourceType::ShaderState; }
        VkPipelineShaderStageCreateInfo shaderStageInfo[K_MAX_SHADER_STAGES];

        std::string name;

        u32 activeShaders = 0;
        bool graphicsPipeline = false;
    };

    struct ShaderStage
    {
        const u32 *code;
        u32 codeSize = 0;
        VkShaderStageFlagBits type = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        const char *entryPointName;
        DescriptorSetLayoutInfo descriptorSetLayoutData;
    };

    struct ShaderStateCreationData
    {
        std::string name;
        ShaderStage stages[K_MAX_SHADER_STAGES];
        u32 stageCount = 0;

        ShaderStateCreationData &SetName(const char *name);
        ShaderStateCreationData &AddStage(const u32 *code, const char *entryPointName, size_t code_size,
                                          VkShaderStageFlagBits type);
        ShaderStage &GetStage(VkShaderStageFlagBits stageFlag);
    };

    class ComputeShader;

    struct ComputePipelineCreationData
    {
        const char *name;
        const char *kernel;
        GPUResourceHandle<ComputeShader> computeShaderHandle;

        DescriptorSetLayoutsCollection descriptorCreationData;
        std::vector<SPtr<DescriptorSetLayout>> *descriptorSetLayouts;
        VkDescriptorSetLayout *vkDescriptorSetLayout;
        u32 activeLayouts;
    };

    struct PushConstantInfo
    {
        std::vector<VkPushConstantRange> ranges;
        VkShaderStageFlags shaderStages;
    };


}

#pragma region Pipeline

#pragma endregion
