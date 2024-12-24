#pragma once
#include "FrameGraph/RenderPass.h"
#include "ComputeShader.h"

namespace Pudu
{
	class ComputeRenderPass :public RenderPass
	{
	public:
		void SetGroupSize(uint32_t x, uint32_t y, uint32_t z);
		void SetKernel(std::string& name);
		void SetShader(SPtr<ComputeShader> shader);
		void Render(RenderFrameData& frameData) override;
		void PreRender(RenderFrameData& frameData) override {};
		void AfterRender(RenderFrameData& frameData) override {};

	private:
		std::string m_kernel;
		uint32_t m_groupX;
		uint32_t m_groupY;
		uint32_t m_groupZ;

		SPtr<ComputeShader> m_shader;
	};
}