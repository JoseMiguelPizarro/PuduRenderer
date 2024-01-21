#pragma once
#include "PuduCore.h"
#include "Mesh.h"
#include "MeshCreationData.h"

namespace Pudu {
	class MeshManager
	{
	public:
		static SPtr<Mesh> AllocateMesh(MeshCreationData data);
	};
}

