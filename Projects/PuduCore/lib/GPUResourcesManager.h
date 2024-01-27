#pragma once
#include <filesystem>
#include "PuduCore.h"
#include "Texture2D.h"


namespace Pudu {
	class PuduGraphics;

	namespace fs = std::filesystem;
	class GPUResourcesManager
	{
	public:
		void Init(PuduGraphics* graphics);
		SPtr<Texture2d> GetTexture(Handle handle);
		SPtr<Texture2d> AllocateTexture(fs::path path);
	private:
		PuduGraphics* m_graphics = nullptr;
		std::vector<SPtr<Texture2d>> m_textures;
	};
}

