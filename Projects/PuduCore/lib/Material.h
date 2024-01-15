#pragma once
#include "Shader.h"
#include <Texture2D.h>

namespace Pudu {

	class Material
	{
	public:
		Shader* Shader;
		Texture2d Texture;

	};
}

