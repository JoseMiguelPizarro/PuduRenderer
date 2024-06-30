#pragma once
#include <GPUCommands.h>
#include "Resources/Resources.h"
#include "RenderFrameData.h"
#include "DrawCall.h"

namespace Pudu
{
	class PuduGraphics;

	enum RenderPassOperation
	{
		DontCare,
		Load,
		Clear,
		Count
	}; // enum Enum


	struct RenderPassAttachment
	{
		SPtr<Texture2d> texture;
		VkAttachmentLoadOp loadOperation;
		VkAttachmentStoreOp storeOp;
		VkClearValue clearValue;
		VkImageLayout layout;

	};

	struct RenderPassCreationData
	{
		bool isCompute;
		ComputeShader* computeShader;
		RenderPassType type;
	};

	struct RenderPassAttachments
	{
		VkFormat depthStencilFormat;

		VkImageLayout depthStencilFinalLayout;

		RenderPassOperation depthOperation = RenderPassOperation::DontCare;
		RenderPassOperation stencilOperation = RenderPassOperation::DontCare;

		RenderPassAttachments& Reset();
		RenderPassAttachments& AddColorAttachment(RenderPassAttachment attachment);
		RenderPassAttachments& SetDepthStencilAttachment(RenderPassAttachment attachment);
		RenderPassAttachments& SetDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil);
		VkFormat GetStencilFormat();

		uint16_t colorAttachmentCount = 0;
		uint16_t numColorFormats = 0;
		uint16_t depthAttachmentCount = 0;

		VkFormat colorAttachmentsFormat[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo colorAttachments[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo depthAttachments[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo stencilAttachments[K_MAX_IMAGE_OUTPUTS];
	};

	class RenderPass
	{
	public:
		VkRenderingInfo GetRenderingInfo(RenderFrameData& data);
		VkRect2D renderArea;

		RenderPassHandle handle;
		VkRenderPass vkHandle;

		bool isCompute;

		virtual void BeginRender(RenderFrameData& data);
		virtual void EndRender(RenderFrameData& data);

		RenderPassAttachments attachments;
		uint16_t dispatchX = 0;
		uint16_t dispatchY = 0;
		uint16_t dispatchZ = 0;

		uint8_t numRenderTargets = 0;

		std::string name;

		void SetName(const char* name);


		virtual Pipeline* GetPipeline(RenderFrameData& frameData, DrawCall& drawcall);
		virtual void PreRender(RenderFrameData& renderData) { }
		virtual void Render(RenderFrameData& frameData);
		virtual void AfterRender(RenderFrameData& frameData) {};
		virtual void OnResize(PuduGraphics& gpu, uint32_t newWidth, uint32_t newHeight) {}
		virtual void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual PipelineCreationData GetPipelineCreationData(RenderFrameData& frameData, DrawCall& drawcall);
		virtual void Initialize(PuduGraphics* gpu) {};
		void SetComputeShader(ComputeShader* shader);
		virtual RenderPassType GetRenderPassType() { return RenderPassType::Color;};
		ComputeShader* GetComputeShader();

	private:
		ComputeShader* m_computeShader;
	};

}
