#include "DrawCall.h"

namespace Pudu
{
	SPtr<Material> DrawCall::GetRenderMaterial()
	{
		if (m_replacementMaterial != nullptr)
		{
			return m_replacementMaterial;
		}

		return MaterialPtr;
	}

	void DrawCall::SetReplacementMaterial(SPtr<Material> material)
	{
		m_replacementMaterial = material;
	}
}
