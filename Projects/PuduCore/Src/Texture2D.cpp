#include "Texture2D.h"
#include "PuduGraphics.h"

namespace Pudu
{
	void Texture2d::Dispose()
	{
		if (!m_disposed)
		{
			PuduGraphics::Instance()->DestroyTexture(*this);

			m_disposed = true;
		}
	}
}
