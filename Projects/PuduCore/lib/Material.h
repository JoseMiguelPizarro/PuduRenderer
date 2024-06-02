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
		std::string Name;
	};
}

