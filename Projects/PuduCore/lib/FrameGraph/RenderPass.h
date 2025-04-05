#pragma once
#include <GPUCommands.h>
#include "Resources/Resources.h"
#include "DrawCall.h"
#include "Pipeline.h"
#include "GPUEnums.h"
#include "Enums/ResourceUsage.h"

namespace Pudu
{
	class PuduGraphics;
	struct RenderFrameData;

	struct RenderPassAttachment
	{
		SPtr<RenderTexture> resource;
		SPtr<GraphicsBuffer> buffer;
		VkAttachmentLoadOp loadOperation;
		VkAttachmentStoreOp storeOp;
		VkClearValue clearValue;
		VkImageLayout layout;
		VkImageLayout finalLayout;
		GPUResourceType::Type type = GPUResourceType::Texture;
		AttachmentAccessUsage usage = AttachmentAccessUsage::Write;
		ResourceUsage resourceUsage;

		RenderPassAttachment() = default;

		RenderPassAttachment(
			SPtr<RenderTexture> rt,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkClearValue clearValue = {},
			VkImageLayout layout = {},
			AttachmentAccessUsage usage = AttachmentAccessUsage::Write) :
			resource(rt), loadOperation(loadOp), storeOp(storeOp), clearValue(clearValue), layout(layout), usage(usage) {};
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
		RenderPassAttachments& SetDepthStencilAttachment(const RenderPassAttachment& attachment);
		RenderPassAttachments& SetDepthStencilOperations(LoadOperation depth, LoadOperation stencil);
		RenderPassAttachments& AddBufferAttachment(SPtr<GraphicsBuffer> buffer, AttachmentAccessUsage usage = AttachmentAccessUsage::Write);

		VkFormat GetStencilFormat();

		uint16_t AttachmentCount();
		uint16_t ColorAttachmentCount();
		uint16_t BufferCount();
		uint16_t colorAttachmentVkCount = 0;
		uint16_t depthAttachmentVkCount = 0;

		std::vector<RenderPassAttachment>* GetColorRenderPassAttachments();

		VkRenderingAttachmentInfo* GetVkColorAttachments();
		VkRenderingAttachmentInfo* GetDepthVkAttachments();
		VkRenderingAttachmentInfo* GetStencilAttachments();


		RenderPassAttachment colorAttachments[K_MAX_IMAGE_OUTPUTS];
		RenderPassAttachment depthAttachments[K_MAX_IMAGE_OUTPUTS];
		RenderPassAttachment stencilAttachments[K_MAX_IMAGE_OUTPUTS];
		RenderPassAttachment bufferAttachments[8];

		VkFormat* GetColorAttachmentsFormat();

	private:
		friend class FrameGraph;
		friend class RenderPass;
		uint16_t depthAttachmentCount = 0;
		uint16_t colorAttachmentCount = 0;
		uint16_t buffersCount = 0;


	private:
		bool m_VkcolorAttachmentsCreated;
		bool m_colorRenderPassAttachmentsCreated;
		bool m_depthAttachmentsCreated;
		bool m_stencilAttachmentsCreated;

		VkRenderingAttachmentInfo m_vkcolorAttachments[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo m_vkDepthAttachments[1];
		VkRenderingAttachmentInfo m_vkStencilAttachments[K_MAX_IMAGE_OUTPUTS];
		VkFormat m_colorAttachmentsFormat[K_MAX_IMAGE_OUTPUTS];


		std::vector <RenderPassAttachment> m_colorRenderPassAttachments;
	};

	struct RenderPassCreationData
	{
		bool isCompute;
		bool isEnabled;
		ComputeShader* computeShader;
		std::string name;
		CullMode cullMode = CullMode::Back;

		RenderPassAttachments attachments;
	};

	class RenderPass :public GPUResource<RenderPass>, std::enable_shared_from_this<RenderPass>
	{
	public:
		VkRenderingInfo GetRenderingInfo(RenderFrameData& data);
		virtual void BeginRender(RenderFrameData& data);
		virtual void EndRender(RenderFrameData& data);
		RenderPass* AddColorAttachment(SPtr<RenderTexture> rt, AttachmentAccessUsage accessUsage = AttachmentAccessUsage::Write, LoadOperation loadOp = LoadOperation::DontCare, vec4 clearColor = vec4(0));
		RenderPass* AddDepthStencilAttachment(SPtr<RenderTexture> rt, AttachmentAccessUsage accessUsage = AttachmentAccessUsage::Write, LoadOperation loadOp = LoadOperation::DontCare, float depthClear = 1.0f, uint32_t stencilClear = 0);
		RenderPass* AddColorAttachment(RenderPassAttachment& attachment);
		RenderPass* AddDepthStencilAttachment(RenderPassAttachment& attachment);
		RenderPass* AddBufferAttachment(SPtr<GraphicsBuffer> buffer, AttachmentAccessUsage usage);
		RenderPass* SetName(const char* name);
		RenderPass* SetCullMode(CullMode cullMode);
		RenderPass* SetColorBlending(VkBlendFactor sourceColor, VkBlendFactor destinationColor, VkBlendOp colorOperation);
		RenderPass* SetAlphaBlending(VkBlendFactor sourceAlpha, VkBlendFactor destinationAlpha, VkBlendOp alphaOperation);
		RenderPass* SetReplacementMaterial(SPtr<Material> material);
		RenderPass* SetRenderLayer(uint32_t layer);
		SPtr<Material> GetReplacementMaterial() const;
		bool HasReplacementMaterial() const;

		static void BindPipeline(const Pipeline* pipeline,RenderFrameData& frameData);


		BlendState* GetBlendState();

		CullMode GetCullMode();

		virtual Pipeline* GetPipeline(PipelineQueryData pipelineQuery);

		//Best place for doing any modification to the camera
		virtual void PreRender(RenderFrameData& renderData);
		//Here is where scissors, viewport, depthbias. target render target are set and Camera values are pushed
		virtual void SetupRender(RenderFrameData& frameData);
		virtual void Render(RenderFrameData& frameData);

		//Restore or release resources
		virtual void AfterRender(RenderFrameData& frameData) {};

		virtual void OnResize(PuduGraphics& gpu, uint32_t newWidth, uint32_t newHeight) {}
		virtual void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual void Initialize(PuduGraphics* gpu) {};
		void SetComputeShader(ComputeShader* shader);
		ComputeShader* GetComputeShader();

	protected:
		void OnCreate(PuduGraphics* gpu) override;
		static void BindMaterialDescriptorSets(Pipeline* pipeline,SPtr<Material> material, RenderFrameData& frameData);
		SPtr<Material> GetRenderMaterial(const RenderFrameData& frameData) const;

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
		CullMode m_cullMode = CullMode::Back;
		BlendState m_blendState;
		ComputeShader* m_computeShader;
		uint32_t m_renderLayer;
		SPtr<Material> m_replacementMaterial;
	};

}
