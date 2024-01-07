#pragma once
#include "Mesh.h"
#include "Texture2D.h"

namespace Pudu
{
	class DrawCall
	{
	public:
		Mesh* GetMesh() const { return m_mesh; }
		Texture2d* GetTexture() const { return m_texture; }

		DrawCall() {}
		DrawCall(Mesh* mesh, Texture2d* texture) : m_mesh(mesh), m_texture(texture)
		{
		}

	private:
		Mesh* m_mesh = nullptr;
		Texture2d* m_texture = nullptr;
	};
}
