#include "Resources/Resources.h"
#include "Resources/RenderPassCreationData.h"

namespace Pudu
{
	BlendState& BlendStateCreation::AddBlendState()
	{
		blendStates[activeStatesCount] = {};

		return blendStates[activeStatesCount++];
	}
	BlendState& BlendState::SetColorBlending(VkBlendFactor sourceColor, VkBlendFactor destinationColor, VkBlendOp colorOperation)
	{
		sourceColorFactor = sourceColor;
		destinationColorFactor = destinationColor;
		colorBlendOperation = colorOperation;

		return *this;
	}
	BlendState& BlendState::SetAlphaBlending(VkBlendFactor sourceAlpha, VkBlendFactor destinationAlpha, VkBlendOp alphaOperation)
	{
		sourceAlphaFactor = sourceAlpha;
		destinationAlphaFactor = destinationAlpha;
		alphaBlendOperation = alphaOperation;

		return *this;
	}
	BlendState& BlendState::SetColorWriteMask(ColorWriteEnabled::Mask value)
	{
		colorWriteMask = value;
		return *this;
	}
	DepthStencilCreation& DepthStencilCreation::SetDepth(bool write, VkCompareOp comparisonTest)
	{
		isDepthEnabled = 1;
		depthComparison = comparisonTest;

		return *this;
	}
	
	RenderPassOutput& RenderPassOutput::Reset()
	{
		numColorFormats = 0;
		for (uint32_t i = 0; i < K_MAX_IMAGE_OUTPUTS; ++i) {
			colorFormats[i] = VK_FORMAT_UNDEFINED;
			colorFinalLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;
			colorOperations[i] = RenderPassOperation::DontCare;
		}
		depthStencilFormat = VK_FORMAT_UNDEFINED;
		depthOperation = stencilOperation = RenderPassOperation::DontCare;
		return *this;
	}


	RenderPassOutput& RenderPassOutput::AddColorOutput(VkFormat format, VkImageLayout layout, RenderPassOperation loadOp)
	{
		colorFormats[numColorFormats] = format;
		colorOperations[numColorFormats] = loadOp;
		colorFinalLayouts[numColorFormats++] = layout;

		return *this;
	}
	RenderPassOutput& RenderPassOutput::SetDepthOutput(VkFormat format, VkImageLayout layout)
	{
		depthStencilFormat = format;
		depthStencilFinalLayout = layout;

		return *this;
	}
	RenderPassOutput& RenderPassOutput::SetDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil)
	{
		depthOperation = depth;
		stencilOperation = stencil;

		return *this;
	}
	RenderPassOutput RenderPassOutput::GetFromCreationData(PuduGraphics* graphics, RenderPassCreationData const& creationData)
	{
		RenderPassOutput output{};
		output.Reset();

		for (uint32_t i = 0; i < creationData.numRenderTargets; i++)
		{
			output.AddColorOutput(creationData.colorFormats[i], creationData.colorFinalLayouts[i], creationData.colorOperations[i]);
		}
		if (creationData.depthStencilFormat != VK_FORMAT_UNDEFINED)
		{
			output.SetDepthOutput(creationData.depthStencilFormat, creationData.depthStencilFinalLayout);
		}

		output.depthOperation = creationData.depthOperation;
		output.stencilOperation = creationData.stencilOperation;

		return output;
	}


}
