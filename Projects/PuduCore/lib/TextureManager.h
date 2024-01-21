#pragma once
#include <memory>
#include <filesystem>
#include "Texture2D.h"

namespace fs = std::filesystem;

namespace Pudu {
	class TextureManager
	{
	public:
		//For now just allocate all textures, in a future should cached them based on the path or info
		static std::shared_ptr<Texture2d> AllocateTexture(fs::path path);
	};
}

