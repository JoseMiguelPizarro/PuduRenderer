#pragma once
#include <memory>
#include "PuduCore.h"
#include "Shader.h"
#include "Texture.h"
#include "Texture2D.h"
#include <Pipeline.h>
#include <unordered_map>

#include "GraphicsBuffer.h"
#include "GPUCommands.h"

namespace Pudu
{
    class PuduGraphics;

    class ShaderPropertyType
    {
    public:
        enum Enum
        {
            Vec2,
            Texture,
            Buffer,
            TextureArray,
            Float
        };
    };

    struct ShaderProperty
    {
        std::string name;
        ShaderPropertyType::Enum type;
        vec4 value;
        SPtr<Texture> texture;
        Size arrayIndex;
        SPtr<GraphicsBuffer> buffer;
        std::vector<SPtr<Texture>>* textureArray;
    };

    struct PropertyUpdateRequest
    {
        ShaderProperty property;
        DescriptorBinding* binding;
    };

    struct MaterialApplyPropertyGPUTarget
    {
        PuduGraphics* graphics;
        IDescriptorProvider* descriptorProvider;
        GPUCommands* commands;

        VkDescriptorSet GetDescriptorSet(Size slotIndex) const;

        VkDescriptorSet* m_descriptorSets;
        //Remap absolute layout index to material allocated layout index
        Size* m_descriptorSetRemap = nullptr;
    private:
        friend class Material;
        friend class Shader;
    };

    struct ShaderNodeResourcesAllocationInfo
    {
        ShaderNode* rootNode;
        PuduGraphics* graphics;
        const char* scope;
    };

    class ShaderPropertiesBlock
    {
    public:
        void SetProperty(const std::string_view& name, float value);
        void SetProperty(const std::string& name, vec2 value);
        void SetProperty(const std::string& name, const SPtr<Texture>& texture);
        void SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer);
        void SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray);
        void ApplyProperties(const MaterialApplyPropertyGPUTarget& target);
        std::vector<SPtr<GPUResourceBase>>* GetAllocatedResources();


    private:
        friend class Material;
        std::vector<PropertyUpdateRequest> m_descriptorUpdateRequests;


        void ApplyTexture(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
        void ApplyBuffer(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
        void ApplyTextureArray(PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& target);
        void ApplyVectorValue(const PropertyUpdateRequest& request, const MaterialApplyPropertyGPUTarget& settings);
        void ApplyFloatValue(const PropertyUpdateRequest& value, const MaterialApplyPropertyGPUTarget& target);

        //Recursively allocates resources present in the shader node layout
        void AllocateGPUResourcesFromShaderNode(ShaderNodeResourcesAllocationInfo& allocationInfo);
        void BindPropertyToShaderNode(ShaderNode* node, const ShaderProperty& property);
        ShaderProperty* FetchShaderNodeProperty(const ShaderNode* node);

        std::vector<SPtr<GPUResourceBase>> m_allocatedResources;
        std::unordered_map<const ShaderNode*, ShaderProperty> m_propertiesByShaderNodeMap;
    };

    class Material final : public GPUResource<Material>
    {
    public:
        explicit Material(PuduGraphics* graphics);
        void SetShader(const SPtr<IShaderObject>& shader);
        void SetDescriptorProvider(const SPtr<IDescriptorProvider>& descriptorProvider);
        SPtr<IShaderObject> GetShader() { return m_shader; }
        ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
        void ApplyProperties();
        void SetProperty(const std::string_view& name, float value);
        void SetProperty(const std::string& name, glm::vec2 value);
        void SetProperty(const std::string& name, const SPtr<Texture>& texture);
        void SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer);
        void SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray);
        Material* SetScope(const char* scope);
        Size GetDescriptorSetsCount() const;

        VkDescriptorSet* GetDescriptorSets() { return m_descriptorSets; };

    private:
        friend class PuduGraphics;
        friend class RenderPass;
        SPtr<IShaderObject> m_shader;
        ShaderPropertiesBlock m_propertiesBlock;
        SPtr<IDescriptorProvider> m_descriptorProvider;
        VkDescriptorSet m_descriptorSets[K_MAX_DESCRIPTOR_SET_LAYOUTS]{};
        Size m_descriptorSetsIndexRemap[K_MAX_DESCRIPTOR_SET_LAYOUTS]{};
        Size m_descriptorSetCount = 0;
        bool m_resourcesAllocated = false;
        std::string m_scope;


        void AllocateDescriptorSetsResources(const SPtr<IDescriptorProvider>& descriptorProvider);
    };
}
