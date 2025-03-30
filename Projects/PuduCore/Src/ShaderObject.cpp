#include "ShaderObject.h"

namespace Pudu
{
	ShaderNode* IShaderObject::GetShaderLayout()
	{
		return m_compilationObject.descriptorsData.GetShaderLayout();
	}
}
