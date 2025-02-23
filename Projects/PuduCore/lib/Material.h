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
			Vec2,
			Texture,
			Buffer,
			TextureArray,
			Float
		};
	};

	struct PropertyUpdateRequest {
		std::string name;
		glm::vec4 value;
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
		void SetProperty(const std::string_view& name, float value);
		void SetProperty(const std::string& name, glm::vec2 value);
		void SetProperty(const std::string& name, const SPtr<Texture>& texture);
		void SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer);
		void SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray);
		void ApplyProperties(const MaterialApplyPropertyGPUTarget& target);

	private:
		std::vector<PropertyUpdateRequest> m_descriptorUpdateRequests;

		static void ApplyTexture(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
		static void ApplyBuffer(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
		static void ApplyTextureArray(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
		static void ApplyVectorValue(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
		static void ApplyFloatValue(const PropertyUpdateRequest& value, const MaterialApplyPropertyGPUTarget& target);
	};

	class Material final :public GPUResource<Material>
	{
	public:
		explicit Material(PuduGraphics* graphics);
		std::string Name;
		void SetShader(SPtr<Shader> shader);
		SPtr<Shader> GetShader(){return m_shader;}
		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
		void ApplyProperties();
		void SetProperty(const std::string_view& name, float value);
		void SetProperty(const std::string& name, glm::vec2 value);
		void SetProperty(const std::string& name, const SPtr<Pudu::Texture>& texture);
		void SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer);
		void SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray);
		VkDescriptorSet* GetDescriptorSets() {return m_descriptorSets;};

	private:
		friend class PuduGraphics;
		friend class RenderPass;
		SPtr<Shader> m_shader;
		ShaderPropertiesBlock m_propertiesBlock;
		VkDescriptorSet m_descriptorSets[K_MAX_DESCRIPTOR_SET_LAYOUTS]{};

	};
}

