#include "Texture.h"
#include "PuduGraphics.h"

namespace Pudu {

	void Texture::Destroy()
	{
		if (!m_disposed)
		{
			m_disposed = true;
		}
		else
		{
			LOG("Trying to dispose an already disposed texture {}", name);
		}
	}
	bool Texture::IsDestroyed()
	{
		return m_disposed;
	}
}