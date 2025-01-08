#include "DrawCall.h"

namespace Pudu
{

	Material* DrawCall::GetRenderMaterial()
	{
		if (m_replacementMaterial != nullptr)
		{
			return m_replacementMaterial.get();
		}

		return MaterialPtr;
	}

	void DrawCall::SetReplacementMaterial(SPtr<Material> material)
	{
		m_replacementMaterial = material;
	}
}
