#include "FileManager.h"
#include "PostProcessingRenderPass.h"
#include <filesystem>

namespace Pudu
{
    const static std::vector<Vertex> quadVertices = {
        {.pos = {-.5f, -.5f, 0}, .texcoord = {0.f, 0.f}},
        {.pos = {-.5f, .5f, 0.f}, .texcoord = {0.f, 1.f}},
        {.pos = {.5f, 0.5f, 0.f}, .texcoord = {1.f, 1.f}},
        {.pos = {.5f, -0.5f, 0.f}, .texcoord = {1.f, 0.f}}
    };

    const static std::vector<uint32_t> quadIndices = {
        0, 1, 2,
        0, 2, 3
    };

    void PostProcessingRenderPass::Initialize(PuduGraphics* gpu)
    {
        m_material = gpu->Resources()->AllocateMaterial();

        vec3* positions = new vec3[4]
        {
            {-.5f, -.5f, 0.f}, {-.5f, .5f, .0f}, {.5f, .5f, .0f}, {.5f, -.5f, .0f}
        };

        vec2* texCoords = new vec2[4]{{0, 0}, {0, 1.f}, {1.f, 1.f}, {1.f, 0.f}};

        VertexAttributeStream positionStream;
        positionStream.Attribute.format = ChannelFormat::R32G32B32_SFLOAT;
        positionStream.Attribute.type = VertexAttributeType::POSITION;
        positionStream.Data = positions;
        positionStream.Stride = sizeof(vec3);
        positionStream.Count = 4;

        VertexAttributeStream texCoordStream;
        texCoordStream.Attribute.format = ChannelFormat::R32G32_SFLOAT;
        texCoordStream.Attribute.type = VertexAttributeType::TEXCOORD0;
        texCoordStream.Count = 4;
        texCoordStream.Stride = sizeof(vec2);
        texCoordStream.Data = texCoords;

        std::vector<VertexAttributeStream> attributesStream = { positionStream, texCoordStream};

        MeshCreationData meshCreation = {
            .Name = "quad",
            .Indices = quadIndices,
        };

        meshCreation.VertexAttributeStreams = attributesStream;

        m_quadMesh = gpu->CreateMesh(meshCreation);

        m_screenColor = gpu->GetRenderTexture();
        m_screenColor->name = "ScreenColor";
        m_screenColor->bindless = true;
        m_screenColor->width = gpu->WindowWidth;
        m_screenColor->height = gpu->WindowHeight;
        m_screenColor->depth = 1;
        m_screenColor->format = VK_FORMAT_R8G8B8A8_UNORM;
        m_screenColor->Create(gpu);

        m_postProcessingShader = gpu->CreateShader(m_shaderPath, "Postprocessing");
        m_postProcessingShader->GetShaderLayout()->Print();
        m_material->SetShader(m_postProcessingShader);
        m_material->SetProperty("Data.colorTex", m_screenColor);
    }

    void PostProcessingRenderPass::Render(RenderFrameData& renderData)
    {
        auto command = renderData.currentCommand;

        command->BindMesh(m_quadMesh.get(), m_material->GetShader()->GetVertexLayout());
        Pipeline* pipeline = GetPipeline({
            .renderPass = this,
            .shader = m_postProcessingShader.get(),
            .renderer = renderData.renderer,
        });

        m_material->ApplyProperties(command.get());
        command->BindPipeline(pipeline);

        BindMaterialDescriptorSets(pipeline, m_material, renderData);

        command->DrawIndexed(m_quadMesh->GetIndices()->size());
    }

    void PostProcessingRenderPass::PreRender(RenderFrameData& renderData)
    {
        auto command = renderData.currentCommand;
        auto frameColor = attachments.colorAttachments[0].resource;

        command->TransitionTextureLayout(m_screenColor, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        command->TransitionTextureLayout(frameColor, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        command->Blit(attachments.colorAttachments[0].resource, m_screenColor, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        command->TransitionTextureLayout(m_screenColor, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        command->TransitionTextureLayout(frameColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    void PostProcessingRenderPass::SetExposure(float exposure)
    {
        m_material->SetProperty("Data.exposure", exposure);
    }
}
