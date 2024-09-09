#pragma once
#include <memory>
#include "PuduCore.h"
#include "Shader.h"
#include "Texture2D.h"

namespace Pudu {

	class Material
	{
	public:
		SPtr<Shader> Shader;
		SPtr<Texture2d> Texture;
		SPtr<Texture2d> NormalMap;
		std::string Name;
	};
}

