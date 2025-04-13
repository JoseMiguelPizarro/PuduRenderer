#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "SPIRVParser.h"
#include "ForwardRenderPass.h"
#include "DepthStencilRenderPass.h"
#include "PostProcessingRenderPass.h"
#include "BlitRenderPass.h"
#include "ComputeRenderPass.h"
#include <Logger.h>
#include "Shader.h"
#include <DrawIndirectRenderPass.h>


#include "GlobalConstants.h"
#include "Lighting/LightBuffer.h"
#include "FileManager.h"


namespace Pudu
{
    void PuduRenderer::OnInit(PuduGraphics* graphics, PuduApp* app)
    {
        this->graphics = graphics;
        this->app = app;

        m_globalPropertiesMaterial = graphics->Resources()->AllocateMaterial();
        m_globalPropertiesMaterial->name = "Global Properties Material";
        //Load Globals
        m_globalDescriptorSetLayouts = std::make_shared<DescriptorSetLayoutsCollection>(
            graphics->CreateDescriptorSetLayoutsFromModule("PuduGraphicsModule.slang"));
        m_globalPropertiesMaterial
            ->SetScope("Global")
            ->SetDescriptorProvider(m_globalDescriptorSetLayouts);

        InitLightingBuffer(graphics);
        InitConstantsBuffer(graphics);

        auto depthRT = graphics->GetRenderTexture();
        depthRT->depth = 1;
        depthRT->width = graphics->WindowWidth;
        depthRT->height = graphics->WindowHeight;
        depthRT->format = VK_FORMAT_D32_SFLOAT;
        depthRT->name = "DepthPrepassTexture";
        depthRT->SetUsage(static_cast<ResourceUsage>(DEPTH_WRITE | PIXEL_SHADER_RESOURCE));

        auto depthCopyRT = graphics->GetRenderTexture();
        depthCopyRT->depth = 1;
        depthCopyRT->width = graphics->WindowWidth;
        depthCopyRT->height = graphics->WindowHeight;
        depthCopyRT->format = VK_FORMAT_D32_SFLOAT;
        depthCopyRT->name = "DepthPrepassCopyTexture";
        depthCopyRT->SetUsage(SHADER_RESOURCE);

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
        colorRT->SetUsage(ResourceUsage::RENDER_TARGET);

        auto normalRT = graphics->GetRenderTexture();
        normalRT->depth = 1;
        normalRT->width = graphics->WindowWidth;
        normalRT->height = graphics->WindowHeight;
        normalRT->format = VK_FORMAT_R8G8B8A8_UNORM;
        normalRT->name = "ForwardNormal";

        auto colorCopyRT = graphics->GetRenderTexture();
        colorCopyRT->depth = 1;
        colorCopyRT->width = graphics->WindowWidth;
        colorCopyRT->height = graphics->WindowHeight;
        colorCopyRT->format = VK_FORMAT_R8G8B8A8_UNORM;
        colorCopyRT->name = "ColorCopy";

        m_depthRenderPass = graphics->GetRenderPass<DepthPrepassRenderPass>();
        m_depthRenderPass->name = "DepthPrepassRenderPass";
        m_depthRenderPass->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Write, LoadOperation::Clear);

        m_shadowMapRenderPass = graphics->GetRenderPass<ShadowMapRenderPass>();
        m_shadowMapRenderPass->name = "ShadowMapRenderPass";
        m_shadowMapRenderPass->AddDepthStencilAttachment(shadowRT, AttachmentAccessUsage::Write, LoadOperation::Clear);

        m_forwardRenderPass = graphics->GetRenderPass<ForwardRenderPass>();
        m_forwardRenderPass
            ->SetName("ForwardRenderPass")
            ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Clear)
            ->AddColorAttachment(shadowRT, AttachmentAccessUsage::Read, LoadOperation::Load)
            ->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Read, LoadOperation::Load);


        auto transparentRP = graphics->GetRenderPass<ForwardRenderPass>();
        transparentRP->SetName("Transparent")
                     ->SetRenderLayer(1)
                     ->SetColorBlending(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
                     ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load)
                     ->AddColorAttachment(shadowRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddColorAttachment(normalRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddColorAttachment(colorCopyRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddColorAttachment(depthCopyRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Write, LoadOperation::Load);

        auto overlayRP = graphics->GetRenderPass<ForwardRenderPass>();
        overlayRP
            ->SetName("Overlay")
            ->SetRenderLayer(2)
            ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load)
            ->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::ReadAndWrite, LoadOperation::Clear);

        SPtr<Shader> grassShader = graphics->CreateShader("grass.shader.slang", "Grass");
        auto normalShader = graphics->CreateShader("normals.slang", "Normals");
        auto normalMaterial = graphics->Resources()->AllocateMaterial();
        normalMaterial->SetShader(normalShader);
        auto normalRP = graphics->GetRenderPass<ForwardRenderPass>();
        normalRP->SetName("Normal");
        normalRP->AddColorAttachment(normalRT, AttachmentAccessUsage::Write, LoadOperation::Clear);
        normalRP->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Read, LoadOperation::Load);
        normalRP->SetReplacementMaterial(normalMaterial);


        const uint32_t grassCount = 1;

        auto computeRP = graphics->GetRenderPass<ComputeRenderPass>();
        computeRP.get()->SetName("Grass Compute");
        auto compute = graphics->CreateComputeShader("testCompute.compute.slang", "Test Compute");


        auto grassPointCloud = FileManager::LoadPointCloud("models/Diorama_Cat/CatDiorama_Grass.xyz");
        const uint32_t instances = grassPointCloud.size();
        auto groupSize = ceil(sqrt(instances / (32 * 32)));
        computeRP->SetGroupSize(groupSize, groupSize, 1);

        auto grassBuffer = graphics->CreateGraphicsBuffer(sizeof(glm::vec4) * instances, grassPointCloud.data(),
                                                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Data.GrassPos");

        computeRP->SetShader(compute);
        computeRP->AddBufferAttachment(grassBuffer, AttachmentAccessUsage::Write);

        auto forwardColorCopyRP = graphics->GetRenderPass<BlitRenderPass>();
        forwardColorCopyRP->SetBlitTargets(colorRT, colorCopyRT);
        forwardColorCopyRP->SetName("ForwardColorCopy");


        auto depthCopyRP = graphics->GetRenderPass<BlitRenderPass>();
        depthCopyRP->SetBlitTargets(depthRT, depthCopyRT);

        std::array<VkDrawIndirectCommand, grassCount> indirectCommands{};
        for (size_t i = 0; i < grassCount; i++)
        {
            const uint32_t bladesStripe = 6;
            auto indirectData = &indirectCommands[i];
            indirectData->firstInstance = 0;
            indirectData->firstVertex = 0;
            indirectData->vertexCount = 3 * bladesStripe * 2 + 3;
            indirectData->instanceCount = instances;
        }

        auto indirectBuffer = graphics->CreateGraphicsBuffer(sizeof(VkDrawIndirectCommand) * indirectCommands.size(),
                                                             indirectCommands.data(),
                                                             VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
                                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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
                   ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load)
                   ->AddColorAttachment(shadowRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                   ->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Write, LoadOperation::Load)
                   ->SetName("Grass indirect");

        m_postProcessingRenderPass = graphics->GetRenderPass<PostProcessingRenderPass>();
        m_postProcessingRenderPass->name = "Postprocessing";
        m_postProcessingRenderPass->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load);
        m_postProcessingRenderPass->AddColorAttachment(depthRT, AttachmentAccessUsage::Read, LoadOperation::Load);

        m_imguiRenderPass = graphics->GetRenderPass<ImguiRenderPass>();
        m_imguiRenderPass->name = "ImGui";
        m_imguiRenderPass->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load);

        // AddRenderPass(computeRP.get());
        AddRenderPass(m_depthRenderPass.get());
      //  AddRenderPass(m_shadowMapRenderPass.get());
       // AddRenderPass(normalRP.get());
        AddRenderPass(m_forwardRenderPass.get());
       // AddRenderPass(drawGrassRP.get());
       // AddRenderPass(forwardColorCopyRP.get());
      //  AddRenderPass(depthCopyRP.get());
     //   AddRenderPass(transparentRP.get());
        //AddRenderPass(m_postProcessingRenderPass.get());
      //  AddRenderPass(overlayRP.get());

        AddRenderPass(m_imguiRenderPass.get());
        frameGraph.AllocateRequiredResources();
        frameGraph.Compile();

        std::printf(frameGraph.ToString().c_str());

        m_globalPropertiesMaterial->SetProperty("GLOBALS.shadowMap", shadowRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.normalBuffer", normalRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.depthBuffer", depthCopyRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.lightingBuffer", m_lightingBuffer);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.constants", m_globalConstantsBuffer);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.colorBuffer", colorCopyRT);
    }

    static bool isFirstFrame = true;

    void PuduRenderer::OnRender(RenderFrameData& data)
    {
        data.globalPropertiesMaterial = m_globalPropertiesMaterial;
        m_globalPropertiesMaterial->ApplyProperties();

        data.descriptorSetOffset = m_globalDescriptorSetLayouts->setsCount;
        isFirstFrame = false;
        UpdateLightingBuffer(data);
        UpdateGlobalConstantsBuffer(data);
    }

    void PuduRenderer::OnUploadCameraData(RenderFrameData& frameData)
    {
        //UpdateGlobalConstantsBuffer(frameData);

        GlobalConstants globalConstants{};
        auto camera = m_renderCamera;

        globalConstants.farPlane = camera->Projection.farPlane;
        globalConstants.nearPlane = camera->Projection.nearPlane;
        globalConstants.cameraPosWS = camera->Transform.GetLocalPosition();
        globalConstants.viewMatrix = camera->GetViewMatrix();
        globalConstants.projectionMatrix = camera->Projection.GetProjectionMatrix();

        const Size offset = offsetof(GlobalConstants, nearPlane);
        const Size size = sizeof(GlobalConstants) - offset;
        const byte* data = reinterpret_cast<byte*>(&globalConstants) + offset;

        frameData.currentCommand->UploadBufferData(m_globalConstantsBuffer.get(), data, size, offset);
        frameData.currentCommand->BufferBarrier(m_globalConstantsBuffer.get(), sizeof(GlobalConstants), 0, 0, 0, 0, 0);
    }

    void PuduRenderer::UpdateLightingBuffer(RenderFrameData& frame) const
    {
        LightBuffer lightBuffer{};
        lightBuffer.lightDirection = {-frame.scene->directionalLight->Direction(), 0.0f};
        lightBuffer.dirLightMatrix = frame.scene->directionalLight->GetLightMatrix();
        lightBuffer.shadowMatrix = frame.scene->directionalLight->GetShadowMatrix();

        frame.currentCommand->UploadBufferData(m_lightingBuffer.get(), reinterpret_cast<const byte*>(&lightBuffer),
                                               sizeof(LightBuffer));

        frame.lightingBuffer = m_lightingBuffer;
    }

    void PuduRenderer::UpdateGlobalConstantsBuffer(const RenderFrameData& frame) const
    {
        GlobalConstants globalConstants{};
        auto camera = m_renderCamera;
        const auto graphics = frame.graphics;
        globalConstants.screenSize = {graphics->WindowWidth, graphics->WindowHeight};
        globalConstants.time = frame.app->Time.Time();
        globalConstants.farPlane = camera->Projection.farPlane;
        globalConstants.nearPlane = camera->Projection.nearPlane;
        globalConstants.cameraPosWS = camera->Transform.GetLocalPosition();
        globalConstants.viewMatrix = camera->GetViewMatrix();
        globalConstants.projectionMatrix = camera->Projection.GetProjectionMatrix();

        frame.currentCommand->UploadBufferData(m_globalConstantsBuffer.get(),
                                               reinterpret_cast<const byte*>(&globalConstants),
                                               sizeof(GlobalConstants));

        frame.currentCommand->BufferBarrier(m_globalConstantsBuffer.get(), sizeof(GlobalConstants), 0, 0, 0, 0, 0);
    }

    void PuduRenderer::InitLightingBuffer(PuduGraphics* graphics)
    {
        m_lightingBuffer = graphics->CreateGraphicsBuffer(sizeof(LightBuffer), nullptr,
                                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                          VMA_ALLOCATION_CREATE_MAPPED_BIT
                                                          , "LightingBuffer");
    }

    void PuduRenderer::InitConstantsBuffer(PuduGraphics* graphics)
    {
        m_globalConstantsBuffer = graphics->CreateGraphicsBuffer(sizeof(GlobalConstants), nullptr,
                                                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                                                 | VMA_ALLOCATION_CREATE_MAPPED_BIT, "GlobalConstants");
    };
}
