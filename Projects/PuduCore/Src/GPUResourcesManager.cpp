#include "GPUResourcesManager.h"
#include "PuduGraphics.h"

namespace Pudu {

	void GPUResourcesManager::Init(PuduGraphics* graphics)
	{
		m_graphics = graphics;
	}

	SPtr<Texture2d> GPUResourcesManager::GetTexture(Handle handle)
	{
		return m_textures[handle];
	}

	SPtr<Texture2d> GPUResourcesManager::AllocateTexture(fs::path path)
	{
		Handle handle =static_cast<Handle>(m_textures.size());
		auto texture = std::make_shared<Texture2d>((m_graphics->CreateTexture(path, handle)));
		m_textures.push_back(texture);

		m_graphics->UpdateTexture(*texture);
		return texture;
	}
}
