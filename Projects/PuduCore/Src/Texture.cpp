#include "Texture.h"
#include "PuduGraphics.h"

namespace Pudu {

	void Texture::Dispose()
	{
		if (!m_disposed)
		{
			PuduGraphics::Instance()->DestroyTexture(*this);

			m_disposed = true;
		}
	}
}