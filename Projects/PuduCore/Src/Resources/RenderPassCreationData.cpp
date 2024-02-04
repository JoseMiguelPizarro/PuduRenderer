#include "Resources/Resources.h"
#include "Resources/RenderPassCreationData.h"

namespace Pudu
{
	RenderPassCreationData& RenderPassCreationData::Reset()
	{
		numRenderTargets = 0;
		depthStencilFormat = VK_FORMAT_UNDEFINED;
		for (uint32_t i = 0;i < K_MAX_IMAGE_OUTPUTS; ++i)
		{
			colorOperations[i] = RenderPassOperation::DontCare;
		}

		depthOperation = stencilOperation = RenderPassOperation::DontCare;

		return *this;
	}
	RenderPassCreationData& RenderPassCreationData::AddAttachment(VkFormat format, VkImageLayout layout, RenderPassOperation loadOp)
	{
		colorFormats[numRenderTargets] = format;
		colorOperations[numRenderTargets] = loadOp;
		colorFinalLayouts[numRenderTargets++] = layout;

		return *this;
	}
	RenderPassCreationData& RenderPassCreationData::SetDepthStencilTexture(VkFormat format, VkImageLayout layout)
	{
		depthStencilFormat = format;
		depthStencilFinalLayout = layout;

		return *this;
	}
	RenderPassCreationData& RenderPassCreationData::SetName(const char* name)
	{
		this->name = name;

		return *this;
	}
	RenderPassCreationData& RenderPassCreationData::SetDepthStencilOperation(RenderPassOperation depth, RenderPassOperation stencil)
	{
		depthOperation = depth;
		stencilOperation = stencil;

		return *this;
	}
}