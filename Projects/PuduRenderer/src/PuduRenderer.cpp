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
    void PuduRenderer::SetSkyBox(SPtr<Texture> skybox)
    {
        m_skybox = skybox;
        m_globalPropertiesMaterial->SetProperty("GLOBALS.skybox", skybox);

        InitIBL(graphics, m_skybox);
    }

    SPtr<Texture> PuduRenderer::GetSkybox()
    {
        return m_skybox;
    }

    SPtr<Texture> PuduRenderer::GetEnvMap()
    {
        return m_envMap;
    }

    SPtr<Texture> PuduRenderer::GetBRDF_LUT()
    {
        return m_BRDF_LUT;
    }

    SPtr<Texture> PuduRenderer::GetIBL()
    {
        return m_IBL;
    }

    SPtr<Texture> PuduRenderer::GetIBLDiffuse()
    {
        return m_IBL_DiffuseCube;
    }

    SPtr<RenderTexture> PuduRenderer::GetDepthCopyRT() const
    {
        return m_depthCopyRT;
    }

    SPtr<RenderTexture> PuduRenderer::GetColorCopyRT() const
    {
        return m_colorCopyRT;
    }

    void PuduRenderer::SetDebugMode(Debug mode)
    {
        m_globalPropertiesMaterial->SetProperty("DEBUG_SETTINGS.debugMode", static_cast<int>(mode));
    }

    void PuduRenderer::SetRoughnessScale(float scale)
    {
        m_globalPropertiesMaterial->SetProperty("DEBUG_SETTINGS.roughnessScale", scale);
    }

    void PuduRenderer::EnablePostProcessing(bool enable) const
    {
        m_postProcessingRenderPass->isEnabled = enable;
    }

    void PuduRenderer::SetGamma(float gamma)
    {
        m_gamma = gamma;
        m_globalPropertiesMaterial->SetProperty("GLOBALS.gamma", m_gamma);
    }

    void PuduRenderer::EnableToneMapping(bool enable) const
    {
        m_globalPropertiesMaterial->SetProperty("DEBUG_SETTINGS.toneMapping", enable);
    }

    void PuduRenderer::SetExposure(float value) const
    {
        m_postProcessingRenderPass->SetExposure(value);
    }

    void PuduRenderer::InitBRDF_LUT(PuduGraphics* gfx)
    {
        u32 brdfLUTResolution = 256;
        SamplerCreationData brdfLUTSamplerCreationData{};
        brdfLUTSamplerCreationData.wrap = false;
        TextureCreationData BRDF_LUTCreationData;
        BRDF_LUTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        BRDF_LUTCreationData.width = brdfLUTResolution;
        BRDF_LUTCreationData.height = brdfLUTResolution;
        BRDF_LUTCreationData.generateMipmaps = false;
        BRDF_LUTCreationData.textureType = TextureType::Texture2D;
        BRDF_LUTCreationData.name = "BRDF_LUT";
        BRDF_LUTCreationData.flags = TextureFlags::UnorderedAccess;
        BRDF_LUTCreationData.samplerData = &brdfLUTSamplerCreationData;
        BRDF_LUTCreationData.layers = 1;

        auto BRDF_LUTHandle = gfx->CreateTexture(BRDF_LUTCreationData);
        m_BRDF_LUT = gfx->Resources()->GetTexture<Texture>(BRDF_LUTHandle);

        ComputeShaderCreationData BRDF_LUT_CS_Data{"Compute/brdfLUT.compute.slang", "BRDF_LUT"};
        auto BRDF_LUTCS = gfx->CreateComputeShader(BRDF_LUT_CS_Data);
        auto BRDF_LUTMat = gfx->Resources()->AllocateMaterial();

        BRDF_LUTMat->SetShader(BRDF_LUTCS);
        BRDF_LUTMat->SetProperty("material.output", m_BRDF_LUT);

        ComputeShaderRenderer brdfLutCSRenderer;
        brdfLutCSRenderer.SetShader(BRDF_LUTCS);
        brdfLutCSRenderer.SetMaterial(BRDF_LUTMat);

        gfx->DispatchCompute(&brdfLutCSRenderer, brdfLUTResolution / 32, brdfLUTResolution / 32, 1);

        m_globalPropertiesMaterial->SetProperty("GLOBALS.BRDF_LUT", m_BRDF_LUT);
    }

    void PuduRenderer::InitIBL(PuduGraphics* gfx, SPtr<Texture> envMap)
    {
        u32 IBLRTResolution = envMap->width >> 2;
        u32 IBLMips = Texture::CalculateMipLevels(IBLRTResolution, IBLRTResolution);

        SamplerCreationData samplerData{};
        TextureCreationData IBLRTCreationData{};
        IBLRTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        IBLRTCreationData.width = IBLRTResolution;
        IBLRTCreationData.height = IBLRTResolution;
        IBLRTCreationData.textureType = TextureType::Texture_2D_Array;
        IBLRTCreationData.generateMipmaps = false;
        IBLRTCreationData.name = "IBLRT";
        IBLRTCreationData.flags = TextureFlags::UnorderedAccess;
        IBLRTCreationData.layers = 6;
        IBLRTCreationData.exposeMipViews = true;
        IBLRTCreationData.mipmaps = IBLMips;
        IBLRTCreationData.samplerData = &samplerData;


        auto IBLRTHandle = gfx->CreateTexture(IBLRTCreationData);
        m_IBL = gfx->Resources()->GetTexture<Texture>(IBLRTHandle);

        ComputeShaderCreationData IBL_ComputeData{"Compute/IBL.compute.slang", "IBL"};
        ComputeShaderRenderer IBL_CSRenderer;
        auto IBLCS = gfx->CreateComputeShader(IBL_ComputeData);
        auto IBLMaterial = gfx->Resources()->AllocateMaterial();
        IBLMaterial->SetShader(IBLCS);
        IBLMaterial->SetProperty("material.input", envMap);
        IBLMaterial->SetProperty("material.inputResolution", envMap->width);
        IBL_CSRenderer.SetShader(IBLCS);
        IBL_CSRenderer.SetMaterial(IBLMaterial);

        for (int mip = 0; mip < m_IBL->mipLevels; mip++)
        {
            uint resolution = m_IBL->width >> mip; //Assuming power of 2 hehe

            float roughness = static_cast<float>(mip) / (m_IBL->mipLevels - 1);
            IBLMaterial->SetProperty("material.output", m_IBL, mip);
            IBLMaterial->SetProperty("material.roughness", roughness);
            IBLMaterial->SetProperty("material.sampleCount", 1024);
            IBLMaterial->SetProperty("material.outputResolution", resolution);

            gfx->DispatchCompute(&IBL_CSRenderer, resolution / 32, resolution / 32, 6);
        }

        if (m_IBL_SpecularCube != nullptr &&
            (m_IBL_SpecularCube->width != IBLRTResolution || m_IBL_SpecularCube->height != IBLRTResolution))
        {
            gfx->DestroyTexture(m_IBL_SpecularCube);
        }

        std::string specularIBLName = ("IBL_SpecularCube_" + envMap->name);
        TextureCreationData envCubemapCreationData{};
        envCubemapCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        envCubemapCreationData.width = IBLRTResolution;
        envCubemapCreationData.height = IBLRTResolution;
        envCubemapCreationData.generateMipmaps = false;
        envCubemapCreationData.textureType = TextureType::Texture_Cube;
        envCubemapCreationData.name = specularIBLName.c_str();
        envCubemapCreationData.layers = 6;
        envCubemapCreationData.samplerData = &samplerData;
        envCubemapCreationData.mipmaps = IBLMips;
        auto envCubemapHandle = gfx->CreateTexture(envCubemapCreationData);
        m_IBL_SpecularCube = gfx->Resources()->GetTexture<Texture>(envCubemapHandle);

        auto cmd = gfx->BeginSingleTimeCommands();
        cmd.Blit(m_IBL, m_IBL_SpecularCube);

        TextureCreationData IBLDiffuseRTCreationData{};
        IBLDiffuseRTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        IBLDiffuseRTCreationData.width = IBLRTResolution;
        IBLDiffuseRTCreationData.height = IBLRTResolution;
        IBLDiffuseRTCreationData.generateMipmaps = false;
        IBLDiffuseRTCreationData.textureType = TextureType::Texture_2D_Array;
        IBLDiffuseRTCreationData.name = "IBL_RT";
        IBLDiffuseRTCreationData.flags = TextureFlags::UnorderedAccess;
        IBLDiffuseRTCreationData.layers = 6;
        IBLDiffuseRTCreationData.samplerData = &samplerData;

        auto IBL_DiffuseRT = gfx->Resources()->GetTexture<Texture>(gfx->CreateTexture(IBLDiffuseRTCreationData));

        ComputeShaderCreationData IBLDiffuse_ComputeData{
            "Compute/IBL.compute.slang", "IBL_Diffuse", "Kernel_DiffuseIBL"
        };
        auto IBL_DiffuseCS = gfx->CreateComputeShader(IBLDiffuse_ComputeData);
        auto IBL_DiffuseMaterial = gfx->Resources()->AllocateMaterial();
        IBL_DiffuseMaterial->SetShader(IBL_DiffuseCS);

        uint skyboxMip = max(1u, static_cast<uint>(std::log2(envMap->width / IBLRTResolution)) + 1);

        IBL_DiffuseMaterial->SetProperty("materialDiffuse.input", envMap, skyboxMip);
        IBL_DiffuseMaterial->SetProperty("materialDiffuse.output", IBL_DiffuseRT);
        IBL_DiffuseMaterial->SetProperty("materialDiffuse.outputResolution", IBLRTResolution);
        IBL_DiffuseMaterial->SetProperty("materialDiffuse.inputResolution", m_skybox->width);
        IBL_DiffuseMaterial->SetProperty("materialDiffuse.sampleCount", 1024);

        ComputeShaderRenderer IBL_DiffuseCSRenderer;
        IBL_DiffuseCSRenderer.SetShader(IBL_DiffuseCS);
        IBL_DiffuseCSRenderer.SetMaterial(IBL_DiffuseMaterial);

        gfx->DispatchCompute(&IBL_DiffuseCSRenderer, IBLRTResolution / 32, IBLRTResolution / 32, 6);


        if (m_IBL_DiffuseCube != nullptr &&
            (m_IBL_DiffuseCube->width != IBLRTResolution || m_IBL_DiffuseCube->height != IBLRTResolution))
        {
            gfx->DestroyTexture(m_IBL_SpecularCube);
        }

        std::string diffuseIBLName = ("IBL_DiffuseCube_" + envMap->name);
        TextureCreationData IBLDiffuseCubeCreationData{};
        IBLDiffuseCubeCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        IBLDiffuseCubeCreationData.width = IBLRTResolution;
        IBLDiffuseCubeCreationData.height = IBLRTResolution;
        IBLDiffuseCubeCreationData.generateMipmaps = false;
        IBLDiffuseCubeCreationData.textureType = TextureType::Texture_Cube;
        IBLDiffuseCubeCreationData.name = diffuseIBLName.c_str();
        IBLDiffuseCubeCreationData.layers = 6;
        IBLDiffuseCubeCreationData.samplerData = &samplerData;

        auto IBLDiffuseCubeHandle = gfx->CreateTexture(IBLDiffuseCubeCreationData);
        m_IBL_DiffuseCube = gfx->Resources()->GetTexture<Texture>(IBLDiffuseCubeHandle);

        cmd.Blit(IBL_DiffuseRT, m_IBL_DiffuseCube);
        gfx->EndSingleTimeCommands(cmd);

        gfx->DestroyTexture(IBL_DiffuseRT);

        m_globalPropertiesMaterial->SetProperty("GLOBALS.IBL_Specular", m_IBL_SpecularCube);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.IBL_Levels", m_IBL_SpecularCube->mipLevels);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.IBL_Diffuse", m_IBL_DiffuseCube);
    }

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
        InitBRDF_LUT(graphics);

        auto depthRT = graphics->GetRenderTexture();
        depthRT->depth = 1;
        depthRT->width = graphics->WindowWidth;
        depthRT->height = graphics->WindowHeight;
        depthRT->format = VK_FORMAT_D32_SFLOAT;
        depthRT->name = "DepthPrepassTexture";
        depthRT->SetUsage(static_cast<ResourceUsage>(DEPTH_WRITE | PIXEL_SHADER_RESOURCE));

        m_depthCopyRT = graphics->GetRenderTexture();

        m_depthCopyRT->depth = 1;
        m_depthCopyRT->width = graphics->WindowWidth;
        m_depthCopyRT->height = graphics->WindowHeight;
        m_depthCopyRT->format = VK_FORMAT_D32_SFLOAT;
        m_depthCopyRT->name = "DepthPrepassCopyTexture";
        m_depthCopyRT->SetUsage(SHADER_RESOURCE);

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

        m_colorCopyRT = graphics->GetRenderTexture();
        m_colorCopyRT->depth = 1;
        m_colorCopyRT->width = graphics->WindowWidth;
        m_colorCopyRT->height = graphics->WindowHeight;
        m_colorCopyRT->format = VK_FORMAT_R8G8B8A8_UNORM;
        m_colorCopyRT->name = "ColorCopy";

        m_depthRenderPass = graphics->GetRenderPass<DepthPrepassRenderPass>();
        m_depthRenderPass->name = "DepthPrepassRenderPass";
        m_depthRenderPass->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Write, LoadOperation::Clear)
                         ->SetMultisampled(true);


        m_shadowMapRenderPass = graphics->GetRenderPass<ShadowMapRenderPass>();
        m_shadowMapRenderPass->name = "ShadowMapRenderPass";
        m_shadowMapRenderPass->AddDepthStencilAttachment(shadowRT, AttachmentAccessUsage::Write, LoadOperation::Clear);

        m_forwardRenderPass = graphics->GetRenderPass<ForwardRenderPass>();
        m_forwardRenderPass
            ->SetName("ForwardRenderPass")
            ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Clear)
            ->AddColorAttachment(shadowRT, AttachmentAccessUsage::Read, LoadOperation::Load)
            ->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Read, LoadOperation::Load)
            ->SetMultisampled(true);


        auto transparentRP = graphics->GetRenderPass<ForwardRenderPass>();
        transparentRP->SetName("Transparent")
                     ->SetRenderLayer(1)
                     ->SetColorBlending(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
                     ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load)
                     ->AddColorAttachment(shadowRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddColorAttachment(normalRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddColorAttachment(m_colorCopyRT, AttachmentAccessUsage::Read, LoadOperation::Load)
                     ->AddColorAttachment(m_depthCopyRT, AttachmentAccessUsage::Read, LoadOperation::Load)
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


        auto grassPointCloud = FileManager::LoadPointCloud("models/Diorama_Cat/CatDiorama_Grass.xyz");
        const uint32_t instances = grassPointCloud.size();

        auto grassBuffer = graphics->CreateGraphicsBuffer(sizeof(glm::vec4) * instances, grassPointCloud.data(),
                                                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Data.GrassPos");


        auto forwardColorCopyRP = graphics->GetRenderPass<BlitRenderPass>();
        forwardColorCopyRP->SetBlitTargets(colorRT, m_colorCopyRT);
        forwardColorCopyRP->SetName("ForwardColorCopy");


        auto depthCopyRP = graphics->GetRenderPass<BlitRenderPass>();
        depthCopyRP->SetBlitTargets(depthRT, m_depthCopyRT);

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

        // drawGrassRP.get()
        //            ->SetMaterial(material)
        //            ->SetOffset(0)
        //            ->SeStride(sizeof(VkDrawIndirectCommand))
        //            ->SetDrawCount(indirectCommands.size())
        //            ->SetIndirectBuffer(indirectBuffer)
        //            ->SetCullMode(CullMode::None)
        //            ->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load)
        //            ->AddColorAttachment(shadowRT, AttachmentAccessUsage::Read, LoadOperation::Load)
        //            ->AddDepthStencilAttachment(depthRT, AttachmentAccessUsage::Write, LoadOperation::Load)
        //            ->SetName("Grass indirect");

        m_postProcessingRenderPass = graphics->GetRenderPass<PostProcessingRenderPass>();
        m_postProcessingRenderPass->name = "Postprocessing";
        m_postProcessingRenderPass->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load);
        m_postProcessingRenderPass->AddColorAttachment(m_depthCopyRT, AttachmentAccessUsage::Read, LoadOperation::Load);

        m_imguiRenderPass = graphics->GetRenderPass<ImguiRenderPass>();
        m_imguiRenderPass->name = "ImGui";
        m_imguiRenderPass->AddColorAttachment(colorRT, AttachmentAccessUsage::Write, LoadOperation::Load);

        // AddRenderPass(computeRP.get());
        AddRenderPass(m_depthRenderPass.get());
        AddRenderPass(m_shadowMapRenderPass.get());
        //  AddRenderPass(normalRP.get());
        AddRenderPass(m_forwardRenderPass.get());
        // AddRenderPass(drawGrassRP.get());
        AddRenderPass(forwardColorCopyRP.get());
        AddRenderPass(depthCopyRP.get());
        AddRenderPass(transparentRP.get());
        AddRenderPass(m_postProcessingRenderPass.get());
        AddRenderPass(overlayRP.get());

        AddRenderPass(m_imguiRenderPass.get());
        frameGraph.AllocateRequiredResources();
        frameGraph.Compile();

        std::printf(frameGraph.ToString().c_str());

        m_globalPropertiesMaterial->SetProperty("GLOBALS.shadowMap", shadowRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.normalBuffer", normalRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.depthBuffer", m_depthCopyRT);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.lightingBuffer", m_lightingBuffer);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.constants", m_globalConstantsBuffer);
        m_globalPropertiesMaterial->SetProperty("GLOBALS.colorBuffer", m_colorCopyRT);

        SetRoughnessScale(1.);
        SetGamma(2.2);
        EnableToneMapping(true);
        SetExposure(2.0);
    }

    static bool isFirstFrame = true;

    void PuduRenderer::OnRender(RenderFrameData& data)
    {
        data.globalPropertiesMaterial = m_globalPropertiesMaterial;
        m_globalPropertiesMaterial->ApplyProperties(data.currentCommand.get());

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
