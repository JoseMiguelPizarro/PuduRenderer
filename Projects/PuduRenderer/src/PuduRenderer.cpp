#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "SPIRVParser.h"
#include "ForwardRenderPass.h"
#include "DepthStencilRenderPass.h"
#include "PostProcessingRenderPass.h"
#include "ComputeRenderPass.h"
#include <Logger.h>
#include "Shader.h"
#include <DrawIndirectRenderPass.h>
#include "Lighting/LightBuffer.h"

namespace Pudu
{
    void PuduRenderer::OnInit(PuduGraphics* graphics, PuduApp* app)
    {
        this->graphics = graphics;
        this->app = app;

        m_globalPropertiesMaterial = graphics->Resources()->AllocateMaterial();

        InitLightingBuffer(graphics);

        auto depthRT = graphics->GetRenderTexture();
        depthRT->depth = 1;
        depthRT->width = graphics->WindowWidth;
        depthRT->height = graphics->WindowHeight;
        depthRT->format = VK_FORMAT_D32_SFLOAT;
        depthRT->name = "DepthPrepassTexture";

        auto shadowRT = graphics->GetRenderTexture();
        shadowRT->depth = 1;
        shadowRT->width = graphics->WindowWidth;
        shadowRT->height = graphics->WindowHeight;
        shadowRT->format = VK_FORMAT_D16_UNORM;
        shadowRT->name = "ShadowMap";

        auto colorRT = graphics->GetRenderTexture();
        colorRT->depth = 1;
        colorRT->width = graphics->WindowWidth;
        colorRT->height = graphics->WindowHeight;
        colorRT->format = VK_FORMAT_R8G8B8A8_UNORM;
        colorRT->name = "ForwardColor";

        auto normalRT = graphics->GetRenderTexture();
        normalRT->depth = 1;
        normalRT->width = graphics->WindowWidth;
        normalRT->height = graphics->WindowHeight;
        normalRT->format = VK_FORMAT_R8G8B8A8_UNORM;
        normalRT->name = "ForwardNormal";

        m_depthRenderPass = graphics->GetRenderPass<DepthPrepassRenderPass>();
        m_depthRenderPass->name = "DepthPrepassRenderPass";
        m_depthRenderPass->AddDepthStencilAttachment(depthRT, AttachmentUsage::Write, LoadOperation::Clear);

        m_shadowMapRenderPass = graphics->GetRenderPass<ShadowMapRenderPass>();
        m_shadowMapRenderPass->name = "ShadowMapRenderPass";
        m_shadowMapRenderPass->AddDepthStencilAttachment(shadowRT, AttachmentUsage::Write, LoadOperation::Clear);

        m_forwardRenderPass = graphics->GetRenderPass<ForwardRenderPass>();
        m_forwardRenderPass
            ->SetName("ForwardRenderPass")
            ->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Clear,vec4(0.4, .4, 0.6, 0.))
            ->AddColorAttachment(shadowRT, AttachmentUsage::Read, LoadOperation::Load)
            ->AddDepthStencilAttachment(depthRT, AttachmentUsage::Read, LoadOperation::Load);

        auto transparentRP = graphics->GetRenderPass<ForwardRenderPass>();
        transparentRP->SetName("Transparent")
                     ->SetRenderLayer(1)
                     ->SetColorBlending(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
                     ->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load)
                     ->AddColorAttachment(shadowRT, AttachmentUsage::Read, LoadOperation::Load);

        auto overlayRP = graphics->GetRenderPass<ForwardRenderPass>();
        overlayRP
        ->SetName("Overlay")
        ->SetRenderLayer(2)
        ->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load)
        ->AddDepthStencilAttachment(depthRT, AttachmentUsage::ReadAndWrite, LoadOperation::Clear);

        SPtr<Shader> grassShader = graphics->CreateShader("grass.shader.slang", "Grass");
        auto normalShader = graphics->CreateShader("normals.slang", "Normals");
        auto normalMaterial = graphics->Resources()->AllocateMaterial();
        normalMaterial->SetShader(normalShader);
        auto normalRP = graphics->GetRenderPass<ForwardRenderPass>();
        normalRP->SetName("Normal");
        normalRP->AddColorAttachment(normalRT, AttachmentUsage::Write, LoadOperation::Clear);
        normalRP->AddDepthStencilAttachment(depthRT, AttachmentUsage::Read, LoadOperation::Load);
        normalRP->SetReplacementMaterial(normalMaterial);


        const uint32_t grassCount = 1;

        auto computeRP = graphics->GetRenderPass<ComputeRenderPass>();
        computeRP.get()->SetName("Grass Compute");
        auto compute = graphics->CreateComputeShader("testCompute.compute.slang", "Test Compute");

        const uint32_t instances = 1000000;
        auto groupSize = ceil(sqrt(instances / (32 * 32)));
        computeRP->SetGroupSize(groupSize, groupSize, 1);
        auto grassBuffer = graphics->CreateGraphicsBuffer(sizeof(glm::vec2) * instances, nullptr,
                                                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Data.GrassPos");

        computeRP->SetShader(compute);
        computeRP->AddBufferAttachment(grassBuffer, AttachmentUsage::Write);


        uint32_t bladesStripe = 6;
        std::array<VkDrawIndirectCommand, grassCount> indirectCommands;
        for (size_t i = 0; i < grassCount; i++)
        {
            auto indirectData = &indirectCommands[i];
            indirectData->firstInstance = 0;
            indirectData->firstVertex = 0;
            indirectData->vertexCount = 3 * bladesStripe * 2 + 3;
            indirectData->instanceCount = instances;
        }

        auto indirectBuffer = graphics->CreateGraphicsBuffer(sizeof(VkDrawIndirectCommand) * indirectCommands.size(),
                                                             indirectCommands.data(),
                                                             VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "indirectBuffer");

        auto drawGrassRP = graphics->GetRenderPass<DrawIndirectRenderPass>();

        SPtr<Material> material = graphics->Resources()->AllocateMaterial();
        material->SetShader(grassShader);
        material->SetProperty("Data.GrassPos", grassBuffer);
        material->SetProperty("Data.shadowMap", shadowRT);

        drawGrassRP.get()
                   ->SetMaterial(material)
                   ->SetOffset(0)
                   ->SeStride(sizeof(VkDrawIndirectCommand))
                   ->SetDrawCount(indirectCommands.size())
                   ->SetIndirectBuffer(indirectBuffer)
                   ->SetCullMode(CullMode::None)
                   ->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load)
                   ->AddColorAttachment(shadowRT, AttachmentUsage::Read, LoadOperation::Load)
                   ->AddDepthStencilAttachment(depthRT, AttachmentUsage::Write, LoadOperation::Load)
                   ->SetName("Grass indirect");

        m_postProcessingRenderPass = graphics->GetRenderPass<PostProcessingRenderPass>();
        m_postProcessingRenderPass->name = "Postprocessing";
        m_postProcessingRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load);

        m_imguiRenderPass = graphics->GetRenderPass<ImguiRenderPass>();
        m_imguiRenderPass->name = "ImGui";
        m_imguiRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load);

        AddRenderPass(computeRP.get());
        AddRenderPass(m_depthRenderPass.get());
        AddRenderPass(m_shadowMapRenderPass.get());
        AddRenderPass(normalRP.get());
        AddRenderPass(m_forwardRenderPass.get());
        AddRenderPass(drawGrassRP.get());
        AddRenderPass(transparentRP.get());
        AddRenderPass(m_postProcessingRenderPass.get());
      //  AddRenderPass(overlayRP.get());

        AddRenderPass(m_imguiRenderPass.get());
        frameGraph.AllocateRequiredResources();
        frameGraph.Compile();

        std::printf(frameGraph.ToString().c_str());

        m_globalPropertiesMaterial->SetShader(grassShader);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.shadowMap", shadowRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.lightingBuffer", m_lightingBuffer);
    }

    static bool isFirstFrame = true;

    void PuduRenderer::OnRender(RenderFrameData& data)
    {
        data.globalPropertiesMaterial = m_globalPropertiesMaterial;

        m_globalPropertiesMaterial->ApplyProperties();

        data.descriptorSetOffset = isFirstFrame ? 0 : 2;
        isFirstFrame = false;
        UpdateLightingBuffer(data);
    }

    void PuduRenderer::UpdateLightingBuffer(RenderFrameData& frame)
    {
        LightBuffer lightBuffer{};
        lightBuffer.lightDirection = {-frame.scene->directionalLight->Direction(), 0.0f};
        lightBuffer.dirLightMatrix = frame.scene->directionalLight->GetLightMatrix();
        lightBuffer.shadowMatrix = frame.scene->directionalLight->GetShadowMatrix();

        frame.graphics->UploadBufferData(m_lightingBuffer.get(), &lightBuffer, sizeof(LightBuffer));

        frame.lightingBuffer = m_lightingBuffer;
    }

    void PuduRenderer::InitLightingBuffer(PuduGraphics* graphics)
    {
        m_lightingBuffer = graphics->CreateGraphicsBuffer(sizeof(LightBuffer), nullptr,
                                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                          VMA_ALLOCATION_CREATE_MAPPED_BIT
                                                          , "LightingBuffer");
    }
}
