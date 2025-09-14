//
// Created by Hojaverde on 9/12/2025.
//

#include "OmnidirectionalShadowmapRenderPass.h"

#include "PuduGraphics.h"
#include "Renderer.h"
#include "RenderFrameData.h"

namespace Pudu
{
    void OmnidirectionalShadowmapRenderPass::OnCreate(PuduGraphics* gfx)
    {
        m_omnidirectionalBuffer = gfx->CreateGraphicsBuffer(sizeof(OmnidirectionalShadowmapData), nullptr,
                                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                            | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                            VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                                            "OmnidirectionalLightBuffer");

        m_omnidirectionalShader = gfx->CreateShader("OmnidirectionalShadowMap.shader.slang", "Omnidirectional Shader");
        m_omnidirectionalMaterial = gfx->CreateMaterial("OmnidirectionalShadowMapMaterial", m_omnidirectionalShader);

        m_omnidirectionalShader->GetShaderLayout()->Print();
    }

    //Convert from projection space to texture space
    void GetTextureSpaceMatrix(float2 p, float size, float4x4& Ma, float4x4& Mb, float4x4& Mc, float4x4& Md)
    {
        float s = size; //Size
        Ma = float4x4({s, 0, 0, 0}, {0, s / 2, 0, 0}, {0, 0, 1, 0}, {p.x, p.y - s / 2, 0, 1});
        Mb = float4x4({s / 2, 0, 0, 0}, {0, s, 0, 0}, {0, 0, 1, 0}, {p.x + s / 2, p.y, 0, 1});
        Mc = float4x4({s, 0, 0, 0}, {0, s / 2, 0, 0}, {0, 0, 1, 0}, {p.x, p.y + s / 2, 0, 1});
        Md = float4x4({s / 2, 0, 0, 0}, {0, s, 0, 0}, {0, 0, 1, 0}, {p.x - s / 2, p.y, 0, 1});
    }

    void OmnidirectionalShadowmapRenderPass::PreRender(RenderFrameData& renderData)
    {
        auto cmd = renderData.currentCommand;

        std::vector<SPtr<LightEntity>> lights;
        renderData.scene->GetEntitiesOfType<LightEntity>(lights);

        int count = 0;
        uint size = 256;

        uint resolution = 4096;
        uint widthCount = resolution / size;

        for (auto& light : lights)
        {
            if (light->GetLightType() == LightType::Point)
            {
                float2 coordinates = {count % widthCount, count / widthCount};
                float4x4 Ta, Tb, Tc, Td;
                GetTextureSpaceMatrix(coordinates, size, Ta, Tb, Tc, Td);

                mat4 mA = Ta * PerspectiveMatrix(125.26438968, 143.98570868 / 125.26438968, 0.1, 100) * translate(
                    mat4(1.0), light->GetTransform().GetLocalPosition()) * toMat4(
                    quat(float3(radians(27.36780516f), radians(180.f), 0)));
                mat4 mB = Tb * PerspectiveMatrix(143.98570868, 125.26438968 / 143.98570868, 0.1, 100) *
                    translate(mat4(1.0), light->GetTransform().GetLocalPosition()) * toMat4(
                        quat(float3(radians(27.36780516f), radians(0.f), radians(90.f))));
                mat4 mC = Tc * PerspectiveMatrix(125.26438968, 143.98570868 / 125.26438968, 0.1, 100) * translate(
                    mat4(1.0), light->GetTransform().GetLocalPosition()) * toMat4(
                    quat(float3(radians(-27.36780516f), radians(270.f), 0)));
                mat4 mD = Td * PerspectiveMatrix(143.98570868, 125.26438968 / 143.98570868, 0.1, 100) *
                    translate(mat4(1.0), light->GetTransform().GetLocalPosition()) * toMat4(
                        quat(float3(radians(-27.36780516f), radians(90.f), radians(90.f))));

                m_data.shadowMatrix[count * 4] = mA;
                m_data.shadowMatrix[count * 4 + 1] = mB;
                m_data.shadowMatrix[count * 4 + 2] = mC;
                m_data.shadowMatrix[count * 4 + 3] = mD;

                count++;
            }
        }

        cmd->UploadBufferData(m_omnidirectionalBuffer.get(), reinterpret_cast<const byte*>(&m_data),
                              sizeof(OmnidirectionalShadowmapData), 0);
    }

    void OmnidirectionalShadowmapRenderPass::Render(RenderFrameData& frameData)
    {
        auto commands = frameData.currentCommand;

        auto drawCalls = frameData.renderer->GetDrawCalls(m_renderLayer);

        std::vector<SPtr<LightEntity>> lights;
        frameData.scene->GetEntitiesOfType<LightEntity>(lights);

        Size lightCount = 0;
        for (auto& light : lights)
        {
            if (light->GetLightType() == LightType::Point)
            {
                for (DrawCall drawCall : *drawCalls)
                {
                    frameData.currentDrawCall = &drawCall;

                    drawCall.SetReplacementMaterial(m_omnidirectionalMaterial);

                    BeforeRenderDrawcall(frameData, drawCall);

                    auto model = drawCall.ModelPtr;
                    auto mesh = drawCall.MeshPtr;

                    auto material = GetRenderMaterial(frameData);
                    ASSERT(mesh != nullptr, "Trying to render null mesh");
                    ASSERT(material != nullptr, "Trying to render null Material for mesh {}", mesh->name.c_str());
                    ASSERT(material->GetShader() != nullptr, "Trying to render mesh material {} with null shader",
                           mesh->name.c_str());

                    Pipeline* pipeline = GetPipeline({
                        .renderPass = frameData.currentRenderPass.get(),
                        .shader = material->GetShader().get(),
                        .renderer = frameData.renderer
                    });

                    BindPipeline(pipeline, frameData);

                    for (const auto& mat : model->Materials)
                    {
                        mat->ApplyProperties(commands.get());
                    }

                    BindMaterialDescriptorSets(pipeline, material, frameData);

                    commands->BindMesh(mesh.get(), material->GetShader()->GetVertexLayout());

                    auto ubo = frameData.graphics->GetUniformBufferObject(drawCall);

                    commands->SetViewport(GetViewport(frameData));
                    commands->PushConstants(pipeline->vkPipelineLayoutHandle,
                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                            sizeof(UniformBufferObject), &ubo);

                    commands->DrawIndexed(static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);

                    AfterRenderDrawcall(frameData, drawCall);
                }
                lightCount++;
            }
        }
    }
}
