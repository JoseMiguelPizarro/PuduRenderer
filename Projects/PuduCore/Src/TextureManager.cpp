#include "PuduGraphics.h"
#include "TextureManager.h"


namespace Pudu {
	std::shared_ptr<Texture2d> TextureManager::AllocateTexture(fs::path path)
	{
		return std::make_shared<Texture2d>(PuduGraphics::Instance()->CreateTexture(path));
	}
}
