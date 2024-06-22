#include "Resources/Resources.h"

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
		isDepthWriteEnable = write;

		return *this;
	}
}
