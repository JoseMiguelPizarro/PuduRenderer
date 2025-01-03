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
			Buffer,
			TextureArray
		};
	};

	struct PropertyUpdateRequest {
		std::string name;
		SPtr<Texture> texture;
		SPtr<GraphicsBuffer> buffer;
		std::vector<SPtr<Texture>>* textureArray;
		DescriptorBinding* binding;
		ShaderPropertyType::Enum type;
	};

	class ShaderPropertiesBlock
	{
	public:
		void SetProperty(std::string name, SPtr<Texture> texture);
		void SetProperty(std::string name, SPtr<GraphicsBuffer> buffer);
		void SetProperty(std::string name, std::vector<SPtr<Texture>>* textureArray);
		void ApplyProperties(PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline);

	private:
		std::vector<PropertyUpdateRequest> m_descriptorUpdateRequests;

		void ApplyTexture(PropertyUpdateRequest& request, PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline);
		void ApplyBuffer(PropertyUpdateRequest& request, PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline);
		void ApplyTextureArray(PropertyUpdateRequest& request, PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline);
	};



	class Material
	{
	public:
		SPtr<Shader> shader;
		std::string Name;

		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
		void SetProperty(std::string name, SPtr<Pudu::Texture> texture);
		void SetProperty(std::string name, SPtr<GraphicsBuffer> buffer);
		void SetProperty(std::string name, std::vector<SPtr<Texture>>* textureArray);

	private:
		friend class PuduGraphics;
		friend class RenderPass;
		ShaderPropertiesBlock m_propertiesBlock;
	};
}

