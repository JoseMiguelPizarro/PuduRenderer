#pragma once
#include <string>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>
#include <vector>
#include "PuduCore.h"
#include "DescriptorSetLayoutData.h"
#include "GPUResource.h"

namespace Pudu
{
	class RenderPass;
	class RenderTexture;
	class Texture;

#pragma region Handles
	typedef uint32_t FrameGraphHandle;
	typedef uint32_t FrameGraphNodeHandle;
	typedef uint32_t ResourceHandle;

	static const uint32_t k_INVALID_HANDLE = 0xffffffff;


#pragma endregion

	enum ResourceState
	{
		RESOURCE_STATE_UNDEFINED = 0,
		RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
		RESOURCE_STATE_INDEX_BUFFER = 0x2,
		RESOURCE_STATE_RENDER_TARGET = 0x4,
		RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
		RESOURCE_STATE_DEPTH_WRITE = 0x10,
		RESOURCE_STATE_DEPTH_READ = 0x20,
		RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
		RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
		RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
		RESOURCE_STATE_STREAM_OUT = 0x100,
		RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
		RESOURCE_STATE_COPY_DEST = 0x400,
		RESOURCE_STATE_COPY_SOURCE = 0x800,
		RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		RESOURCE_STATE_PRESENT = 0x1000,
		RESOURCE_STATE_COMMON = 0x2000,
		RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
		RESOURCE_STATE_SHADING_RATE_SOURCE = 0x8000,
	};


	namespace QueueType {
		enum Enum {
			Graphics, Compute, CopyTransfer, Count
		};

		enum Mask {
			Graphics_mask = 1 << 0, Compute_mask = 1 << 1, CopyTransfer_mask = 1 << 2, Count_mask = 1 << 3
		};

		static const char* s_value_names[] = {
			"Graphics", "Compute", "CopyTransfer", "Count"
		};

		static const char* ToString(Enum e) {
			return ((uint32_t)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
		}
	}


#pragma region RenderPass

	namespace ColorWriteEnabled
	{
		enum Enum
		{
			Red,
			Green,
			Blue,
			Alpha,
			All,
			Count
		};

		enum Mask
		{
			Red_mask = 1 << 0,
			Green_mask = 1 << 1,
			Blue_mask = 1 << 2,
			Alpha_mask = 1 << 3,
			All_mask = Red_mask | Green_mask | Blue_mask | Alpha_mask
		};

		static const char* s_value_names[] = {
			"Red", "Green", "Blue", "Alpha", "All", "Count"
		};

		static const char* ToString(Enum e)
		{
			return ((uint32_t)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
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

		static const char* s_value_names[] = {
			"Wireframe", "Solid", "Point", "Count"
		};

		static const char* ToString(Enum e)
		{
			return ((uint32_t)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
		}
	} // namespace FillMode

	struct StencilOperationState
	{
		VkStencilOp failOp = VK_STENCIL_OP_KEEP;
		VkStencilOp passOp = VK_STENCIL_OP_KEEP;
		VkStencilOp depthFail = VK_STENCIL_OP_KEEP;
		VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
		uint32_t compareMask = 0xff;
		uint32_t writeMask = 0xff;
		uint32_t reference = 0xff;

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

		uint8_t isDepthEnabled : 1;
		uint8_t isDepthWriteEnable : 1;
		uint8_t isStencilEnabled : 1;
		uint8_t pad : 5;

		// Default constructor
		DepthStencilCreation() : isDepthEnabled(0), isDepthWriteEnable(0), isStencilEnabled(0)
		{
		}

		DepthStencilCreation& SetDepth(bool write, VkCompareOp comparisonTest);

		VkStencilOpState GetVkFront() const { return StencilOperationState::GetVkOpState(front); }
		VkStencilOpState GetVkBack() const { return StencilOperationState::GetVkOpState(back); }
	}; // struct DepthStencilCreation

	struct BlendState
	{
		VkBlendFactor sourceColorFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor destinationColorFactor = VK_BLEND_FACTOR_ONE;
		VkBlendOp colorBlendOperation = VK_BLEND_OP_ADD;

		VkBlendFactor sourceAlphaFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor destinationAlphaFactor = VK_BLEND_FACTOR_ONE;
		VkBlendOp alphaBlendOperation = VK_BLEND_OP_ADD;

		ColorWriteEnabled::Mask colorWriteMask = ColorWriteEnabled::All_mask;

		uint8_t blendEnabled : 1;
		uint8_t pad : 6;


		BlendState() : blendEnabled(0)
		{
		}

		BlendState& SetColorBlending(VkBlendFactor sourceColor, VkBlendFactor destinationColor,
			VkBlendOp colorOperation);
		BlendState& SetAlphaBlending(VkBlendFactor sourceAlpha, VkBlendFactor destinationAlpha,
			VkBlendOp alphaOperation);
		BlendState& SetColorWriteMask(ColorWriteEnabled::Mask value);
	}; // struct BlendState

	struct BlendStateCreation
	{
		BlendState blendStates[K_MAX_IMAGE_OUTPUTS];
		uint32_t activeStatesCount = 0;

		BlendState& AddBlendState();
	}; // BlendStateCreation

	//
	//
	struct RasterizationCreation
	{
		VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
		VkFrontFace front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		FillMode::Enum fill = FillMode::Solid;
	}; // struct RasterizationCreation




	struct ComputePipeline {
		VkPipeline vkHandle;
		VkPipelineLayout vkPipelineLayoutHandle;

		VkPipelineBindPoint vkPipelineBindPoint;
		VkDescriptorSet vkDescriptorSet = VK_FALSE;//Just 1 for now, bindless
		const DescriptorSetLayout* descriptorSetLayouts[K_MAX_DESCRIPTOR_SET_LAYOUTS];
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
		VkImageType vktype;

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
			VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
		};
		return s_vk_data[type];
	}

	namespace TextureFlags
	{
		enum Enum
		{
			Default = 1,
			RenderTarget = 2,
			Compute = 4,
			Count = 8,
			Sample = 16,
		};

		enum Mask
		{
			DefaultMask = 1 << 0,
			RenderTargetMask = 1 << 1,
			ComputeMask = 1 << 2
		};

		static const char* s_value_names[] = {
			"Default", "RenderTarget", "Compute", "Count","Sample"
		};

		static const char* ToString(Enum e)
		{
			return ((uint32_t)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
		}
	} // namespace TextureFlags

	namespace TextureFormat
	{
		inline bool IsDepthStencil(VkFormat value)
		{
			return value == VK_FORMAT_D16_UNORM_S8_UINT || value == VK_FORMAT_D24_UNORM_S8_UINT || value ==
				VK_FORMAT_D32_SFLOAT_S8_UINT;
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
				VK_FORMAT_D16_UNORM_S8_UINT && value <= VK_FORMAT_D32_SFLOAT_S8_UINT);
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


	struct SamplerCreationData {
		bool wrap = false;
		uint32_t maxLOD = 1;
	};


	/// <summary>
	/// Struct passed to create texture functions. 
	/// </summary>
	struct TextureCreationSettings
	{
		const char* name = nullptr;
		bool bindless;
		TextureType::Enum textureType = TextureType::Texture2D;
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		SamplerCreationData* samplerData;
	};

	struct TextureCreationData
	{
		uint16_t width = 1;
		uint16_t height = 1;
		uint16_t depth = 1;
		uint8_t mipmaps = 1;
		uint32_t dataSize = -1;
		uint32_t layers = 1;

		TextureFlags::Enum flags = TextureFlags::Default;

		SamplerCreationData samplerData;
		VkFormat format;
		TextureType::Enum textureType = TextureType::Texture2D;
		bool bindless;
		void* pixels = nullptr;
		void* sourceData = nullptr; //ptr to source data, raw texture data if it exists (ie. raw loaded .ktx file) TODO: Remove, this is kinda ugly
		const char* name = nullptr;
		bool allocate = true;
	};

	struct Framebuffer :GPUResource<Framebuffer>
	{
		VkFramebuffer vkHandle;

		GPUResourceHandle<RenderPass> renderPassHandle;

		uint16_t width = 0;
		uint16_t height = 0;

		float scaleX = 1.f;
		float scaleY = 1.f;

		GPUResourceHandle<Texture> colorAttachmentHandles[K_MAX_IMAGE_OUTPUTS];
		GPUResourceHandle<Texture> depthStencilAttachmentHandle;
		uint32_t numColorAttachments;

		uint8_t resize = 0;

		const char* name = nullptr;
	};

	/// <summary>
	/// Represents the data for the all shaders creation in a given pipeline
	/// </summary>
	struct ShaderState :GPUResource<ShaderState>
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo[K_MAX_SHADER_STAGES];

		std::string name;

		uint32_t activeShaders = 0;
		bool graphicsPipeline = false;
	};


	namespace VertexInputRate
	{
		enum Enum
		{
			PerVertex,
			PerInstance,
			Count
		};

		enum Mask
		{
			PerVertex_mask = 1 << 0,
			PerInstance_mask = 1 << 1,
			Count_mask = 1 << 2
		};

		static const char* s_value_names[] = {
			"PerVertex", "PerInstance", "Count"
		};

		static const char* ToString(Enum e)
		{
			return ((uint32_t)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
		}
	} // namespace VertexInputRate

	namespace VertexComponentFormat
	{
		enum Enum
		{
			Float,
			Float2,
			Float3,
			Float4,
			Mat4,
			Byte,
			Byte4N,
			UByte,
			UByte4N,
			Short2,
			Short2N,
			Short4,
			Short4N,
			Uint,
			Uint2,
			Uint4,
			Count
		};

		static const char* s_value_names[] = {
			"Float", "Float2", "Float3", "Float4", "Mat4", "Byte", "Byte4N", "UByte", "UByte4N", "Short2", "Short2N",
			"Short4", "Short4N", "Uint", "Uint2", "Uint4", "Count"
		};

		static const char* ToString(Enum e)
		{
			return ((uint32_t)e < Enum::Count ? s_value_names[(int)e] : "unsupported");
		}
	} // namespace VertexComponentFormat

	static VkFormat VertexFormatToVkFormat(VertexComponentFormat::Enum value)
	{
		static VkFormat s_vk_vertex_formats[VertexComponentFormat::Count] = {
			VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
			/*MAT4 TODO*/VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_FORMAT_R8_SINT, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8_UINT, VK_FORMAT_R8G8B8A8_UINT,
			VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SNORM,
			VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT,
			VK_FORMAT_R32G32B32A32_UINT
		};

		return s_vk_vertex_formats[value];
	}


	struct VertexAttribute
	{
		uint16_t location = 0;
		uint16_t binding = 0;
		uint32_t offset = 0;
		VkFormat format;

		VkFormat GetVkFormat() { return format; }
	};

	//Rate at which an attribute is pulled from the buffer
	struct VertexStream
	{
		uint16_t binding = 0;
		uint16_t stride = 0;
		VertexInputRate::Enum inputRate = VertexInputRate::Count;

		VkVertexInputRate GetVkInputRate() const
		{
			return inputRate == VertexInputRate::PerVertex
				? VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
				: VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
		}
	};

	struct VertexInputCreation
	{
		uint32_t numVertexStreams = 0;
		uint32_t numVertexAttributes = 0;

		VertexStream vertexStreams[K_MAX_VERTEX_STREAMS];
		VertexAttribute vertexAttributes[K_MAX_VERTEX_ATTRIBUTES];

		VertexInputCreation& AddVertexStream(const VertexStream& stream);
		VertexInputCreation& AddVertexAttribute(const VertexAttribute& attribute);
	}; // struct VertexInputCreation

	struct ShaderStage
	{
		std::vector<char>* code;
		uint32_t codeSize = 0;
		VkShaderStageFlagBits type = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		DescriptorSetLayoutData descriptorSetLayoutData;
	};

	struct ShaderStateCreationData
	{
		std::string name;
		ShaderStage stages[K_MAX_SHADER_STAGES];
		uint32_t stageCount = 0;

		ShaderStateCreationData& SetName(const char* name);
		ShaderStateCreationData& AddStage(std::vector<char>* code, size_t code_size, VkShaderStageFlagBits type);
		ShaderStage& GetStage(VkShaderStageFlagBits stageFlag);
	};

	struct PipelineCreationData
	{
		std::vector<char> vertexShaderData;
		std::vector<char> fragmentShaderData;
		DescriptorsCreationData descriptorCreationData;

		RasterizationCreation rasterization;
		DepthStencilCreation depthStencil;
		BlendStateCreation blendState;
		VertexInputCreation vertexInput;
		ShaderStateCreationData shadersStateCreationData;

		GPUResourceHandle<RenderPass> renderPassHandle;

		uint32_t activeLayouts = 0;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		const char* name = nullptr;
	};

	class ComputeShader;
	struct ComputePipelineCreationData {
		const char* name;
		std::vector<char> data;
		DescriptorsCreationData descriptorsCreationData;
		GPUResourceHandle<ComputeShader> computeShaderHandle;
	};
}

#pragma region Pipeline

#pragma endregion
