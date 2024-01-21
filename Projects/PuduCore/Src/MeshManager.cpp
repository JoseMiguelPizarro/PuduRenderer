#include "PuduGraphics.h"
#include "MeshManager.h"


namespace Pudu {
	SPtr<Mesh> MeshManager::AllocateMesh(MeshCreationData data)
	{
		return std::make_shared<Mesh>(PuduGraphics::Instance()->CreateMesh(data));
	}
}
