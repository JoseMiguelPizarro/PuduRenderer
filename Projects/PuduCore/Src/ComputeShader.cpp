#include "ComputeShader.h"

namespace Pudu
{
	void ComputeShader::SetTexture(const char* name, SPtr<Texture> texture)
	{
		m_propertiesBlock.SetProperty(name, texture);
	}
	void ComputeShader::SetBuffer(const char* name, SPtr<GraphicsBuffer> buffer)
	{
		m_propertiesBlock.SetProperty(name, buffer);
	}
}
