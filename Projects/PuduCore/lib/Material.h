#pragma once
#include <memory>
#include "PuduCore.h"
#include "Shader.h"
#include "Texture.h"
#include "Texture2D.h"
#include <Pipeline.h>
#include "GraphicsBuffer.h"
#include "GPUCommands.h"

namespace Pudu {

	class PuduGraphics;

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

	struct MaterialApplyPropertyGPUTarget
	{
		PuduGraphics* graphics;
		IShaderObject* shader;
		VkDescriptorSet* descriptorSets;
		GPUCommands* commands;
	};

	class ShaderPropertiesBlock
	{
	public:
		void SetProperty(std::string name, SPtr<Texture> texture);
		void SetProperty(std::string name, SPtr<GraphicsBuffer> buffer);
		void SetProperty(std::string name, std::vector<SPtr<Texture>>* textureArray);
		void ApplyProperties(MaterialApplyPropertyGPUTarget target);

	private:
		std::vector<PropertyUpdateRequest> m_descriptorUpdateRequests;

		void ApplyTexture(PropertyUpdateRequest& request, MaterialApplyPropertyGPUTarget settings);
		void ApplyBuffer(PropertyUpdateRequest& request, MaterialApplyPropertyGPUTarget settings);
		void ApplyTextureArray(PropertyUpdateRequest& request, MaterialApplyPropertyGPUTarget settings);
	};

	class Material:public GPUResource<Material>
	{
	public:
		Material(PuduGraphics* graphics);
		std::string Name;
		void SetShader(SPtr<Shader> shader);
		SPtr<Shader> GetShader(){return m_shader;}
		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
		void ApplyProperties();
		void SetProperty(std::string name, SPtr<Pudu::Texture> texture);
		void SetProperty(std::string name, SPtr<GraphicsBuffer> buffer);
		void SetProperty(std::string name, std::vector<SPtr<Texture>>* textureArray);
		VkDescriptorSet* GetDescriptorSets() {return m_descriptorSets;};

	private:
		friend class PuduGraphics;
		friend class RenderPass;
		SPtr<Shader> m_shader;
		ShaderPropertiesBlock m_propertiesBlock;
		VkDescriptorSet m_descriptorSets[K_MAX_DESCRIPTOR_SET_LAYOUTS];

	};
}

