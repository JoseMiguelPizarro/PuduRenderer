#pragma once
#include <cstdint>

namespace Pudu
{
	static const uint8_t K_MAX_IMAGE_OUTPUTS = 8;                // Maximum number of images/render_targets/fbo attachments usable.
	static const uint8_t K_MAX_DESCRIPTOR_SET_LAYOUTS = 8;       // Maximum number of layouts in the pipeline.
	static const uint8_t K_MAX_SHADER_STAGES = 5;                // Maximum simultaneous shader stages. Applicable to all different type of pipelines.
	static const uint8_t K_MAX_DESCRIPTORS_PER_SET = 16;         // Maximum list elements for both descriptor set layout and descriptor sets.
	static const uint8_t K_MAX_VERTEX_STREAMS = 16;
	static const uint8_t K_MAX_VERTEX_ATTRIBUTES = 16;

	static const uint32_t K_SUBMIT_HEADER_SENTINEL = 0xfefeb7ba;
	static const uint32_t K_MAX_RESOURCE_DELETIONS = 64;

	enum RenderPassOperation {
		DontCare, Load, Clear, Count
	}; // enum Enum

#pragma region Resources
	typedef uint32_t FrameGraphHandle;
	typedef uint32_t ResourceHandle;


	static const uint32_t k_INVALID_HANDLE = 0xffffffff;

	struct FrameGraphResourceHandle
	{
		FrameGraphHandle index;
	};

	struct BufferHandle {
		ResourceHandle                  index;
	}; // struct BufferHandle

	struct TextureHandle {
		ResourceHandle                  index;
	}; // struct TextureHandle

	struct ShaderStateHandle {
		ResourceHandle                  index;
	}; // struct ShaderStateHandle

	struct SamplerHandle {
		ResourceHandle                  index;
	}; // struct SamplerHandle

	struct DescriptorSetLayoutHandle {
		ResourceHandle                  index;
	}; // struct DescriptorSetLayoutHandle

	struct DescriptorSetHandle {
		ResourceHandle                  index;
	}; // struct DescriptorSetHandle

	struct PipelineHandle {
		ResourceHandle                  index;
	}; // struct PipelineHandle

	struct RenderPassHandle {
		ResourceHandle                  index;
	}; // struct RenderPassHandle

	struct FramebufferHandle {
		ResourceHandle                  index;
	};

	struct FrameGraphNodeHandle
	{
		FrameGraphHandle index;
	};
#pragma endregion
}
