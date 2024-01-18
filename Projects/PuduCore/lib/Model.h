#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Material.h"
#include "Transform.h"
#include "Mesh.h"
#include "string"

namespace Pudu {
	class Model
	{
	public:
		std::string Name;
		Transform Transform;
		std::vector<Mesh> Meshes;
		std::vector<Material> Materials;
		std::vector<VkDescriptorSet> DescriptorSetByFrame;
	};
}
