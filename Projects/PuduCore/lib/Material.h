#pragma once
#include <memory>
#include "PuduCore.h"
#include "Shader.h"
#include "Texture.h"
#include "Texture2D.h"
#include <Pipeline.h>
#include "GraphicsBuffer.h"

namespace Pudu {


	class ShaderPropertyType {
	public:
		enum Enum{
			Texture,
			Buffer
		};
	};

	struct PropertyUpdateRequest {
		std::string name;
		SPtr<Texture> texture;
		SPtr<GraphicsBuffer> buffer;
		DescriptorBinding* binding;
		ShaderPropertyType::Enum type;
	};

	class ShaderPropertiesBlock
	{
	public:
		void SetProperty(std::string name, SPtr<Pudu::Texture> texture);
		void SetProperty(std::string name, SPtr<GraphicsBuffer> buffer);
		void ApplyProperties(PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline);

	private:
		std::vector<PropertyUpdateRequest> m_descriptorUpdateRequests;
	};



	class Material
	{
	public:
		SPtr<Shader> shader;
		SPtr<Texture2d> Texture;
		SPtr<Texture2d> NormalMap;
		std::string Name;

		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
		void SetProperty(std::string name, SPtr<Pudu::Texture> texture);

	private:
		friend class PuduGraphics;
		friend class RenderPass;
		ShaderPropertiesBlock m_propertiesBlock;


	};
}

