#pragma once

#include "Texture.h"

namespace Pudu {

	class TextureCube :public Texture
	{
		public:
		TextureType::Enum GetTextureType() override;
	};
}

