#pragma once
#include "Mesh.h"

namespace Pudu
{
    class DrawCall
    {
    public:
        Mesh* GetMesh() const { return m_mesh; }

        DrawCall(){}
        DrawCall(Mesh* mesh): m_mesh(mesh)
        {
        }

    private:
        Mesh* m_mesh = nullptr;
    };
}
