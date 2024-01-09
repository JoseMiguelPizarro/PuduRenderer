#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Material.h"
#include "Transform.h"
#include "Mesh.h"

namespace Pudu {
	class Model
	{
	public:
		Transform Transform;
		Mesh* Mesh;
		Material Material;
		std::vector<VkDescriptorSet> DescriptorSetByFrame;
	};
}
