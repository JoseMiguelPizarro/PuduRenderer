#pragma once
#include <GPUCommands.h>
#include "Resources/Resources.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "Pipeline.h"
#include "glm/vec4.hpp"

namespace Pudu
{
	class PuduGraphics;

	enum LoadOperation
	{
		DontCare,
		Load,
		Clear,
	}; // enum Enum

	enum AttachmentUsage {
		Read = 1,
		Write = 2,
		ReadAndWrite = 3,
		Sample = 4
	};

	static VkAttachmentLoadOp ToVk(LoadOperation op) {
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

	static VkAttachmentStoreOp ToVk(AttachmentUsage usage) {
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
		default:
			break;
		}

		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}

	struct RenderPassAttachment
	{
		SPtr<RenderTexture> resource;
		VkAttachmentLoadOp loadOperation;
		VkAttachmentStoreOp storeOp;
		VkClearValue clearValue;
		VkImageLayout layout;
		GPUResourceType::Type type = GPUResourceType::Texture;
		AttachmentUsage usage = AttachmentUsage::Write;

		RenderPassAttachment() = default;

		RenderPassAttachment(
			SPtr<RenderTexture> rt,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkClearValue clearValue = {},
			VkImageLayout layout = {},
			AttachmentUsage usage = AttachmentUsage::Write) :
			resource(rt), loadOperation(loadOp), storeOp(storeOp), clearValue(clearValue), usage(usage), layout(layout) {};
	};

	struct RenderPassAttachments
	{
		VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;

		VkImageLayout depthStencilFinalLayout;

		bool writeDepth;

		LoadOperation depthOperation = LoadOperation::DontCare;
		LoadOperation stencilOperation = LoadOperation::DontCare;

		RenderPassAttachments& Reset();
		RenderPassAttachments& AddColorAttachment(RenderPassAttachment attachment);
		RenderPassAttachments& SetDepthStencilAttachment(RenderPassAttachment attachment);
		RenderPassAttachments& SetDepthStencilOperations(LoadOperation depth, LoadOperation stencil);
		VkFormat GetStencilFormat();

		uint16_t AttachmentCount();
		uint16_t colorAttachmentVkCount = 0;
		uint16_t depthAttachmentVkCount = 0;

		uint16_t numColorFormats = 0;

		VkFormat colorAttachmentsFormat[K_MAX_IMAGE_OUTPUTS];

		VkRenderingAttachmentInfo* GetColorAttachments();
		VkRenderingAttachmentInfo* GetDepthAttachments();
		VkRenderingAttachmentInfo* GetStencilAttachments();

		RenderPassAttachment colorAttachments[K_MAX_IMAGE_OUTPUTS];
		RenderPassAttachment depthAttachments[K_MAX_IMAGE_OUTPUTS];
		RenderPassAttachment stencilAttachments[K_MAX_IMAGE_OUTPUTS];

	private:
		friend class FrameGraph;
		friend class RenderPass;
		uint16_t depthAttachmentCount = 0;
		uint16_t colorAttachmentCount = 0;


	private:
		bool m_colorAttachmentsCreated;
		bool m_depthAttachmentsCreated;
		bool m_stencilAttachmentsCreated;

		VkRenderingAttachmentInfo m_vkcolorAttachments[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo m_vkDepthAttachments[1];
		VkRenderingAttachmentInfo m_vkStencilAttachments[K_MAX_IMAGE_OUTPUTS];
	};

	struct RenderPassCreationData
	{
		bool isCompute;
		bool isEnabled;
		ComputeShader* computeShader;
		RenderPassType type;
		std::string name;

		RenderPassAttachments attachments;
	};

	class RenderPass :public GPUResource<RenderPass>, std::enable_shared_from_this<RenderPass>
	{
	public:
		VkRenderingInfo GetRenderingInfo(RenderFrameData& data);
		virtual void BeginRender(RenderFrameData& data);
		virtual void EndRender(RenderFrameData& data);
		void AddColorAttachment(SPtr<RenderTexture> rt, AttachmentUsage usage = AttachmentUsage::Write, LoadOperation loadOp = LoadOperation::DontCare, vec4 clearColor = vec4(0));
		void AddDepthStencilAttachment(SPtr<RenderTexture> rt, AttachmentUsage usage = AttachmentUsage::Write, LoadOperation loadOp = LoadOperation::DontCare, float depthClear = 1.0f, uint32_t stencilClear = 0);
		void AddColorAttachment(RenderPassAttachment& attachment);
		void AddDepthStencilAttachment(RenderPassAttachment& attachment);

		void SetName(const char* name);

		virtual Pipeline* GetPipeline(PipelineQueryData pipelineQuery);
		virtual void PreRender(RenderFrameData& renderData) { }
		virtual void Render(RenderFrameData& frameData);
		virtual void AfterRender(RenderFrameData& frameData) {};
		virtual void OnResize(PuduGraphics& gpu, uint32_t newWidth, uint32_t newHeight) {}
		virtual void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual PipelineCreationData GetPipelineCreationData(RenderFrameData& frameData, DrawCall& drawcall);
		virtual void Initialize(PuduGraphics* gpu) {};
		void SetComputeShader(ComputeShader* shader);
		ComputeShader* GetComputeShader();
		void Create(PuduGraphics* gpu) override;

	public:

		VkRect2D renderArea;

		VkRenderPass vkHandle;

		bool isCompute;
		bool isEnabled = true;
		bool writeDepth;
		RenderPassAttachments attachments;


		uint16_t dispatchX = 0;
		uint16_t dispatchY = 0;
		uint16_t dispatchZ = 0;

		uint8_t numRenderTargets = 0;

		std::string name;


	private:
		friend class PuduGraphics;
		ComputeShader* m_computeShader;
	};

}
