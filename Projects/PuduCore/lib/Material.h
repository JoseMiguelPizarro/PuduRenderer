#pragma once
#include <memory>
#include "PuduCore.h"
#include "Shader.h"
#include "Texture.h"
#include "Texture2D.h"

namespace Pudu {

	struct DescriptorUpdateRequest {
		SPtr<Texture> texture;
		DescriptorBinding* binding;
	};

	class Material
	{
	public:
		SPtr<Shader> Shader;
		SPtr<Texture2d> Texture;
		SPtr<Texture2d> NormalMap;
		std::string Name;

		std::vector<DescriptorUpdateRequest> descriptorUpdateRequests;

		void SetProperty(std::string name, SPtr<Pudu::Texture> texture);
	};
}

