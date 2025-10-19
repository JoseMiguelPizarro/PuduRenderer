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
        m_omnidirectionalShader = gfx->CreateShader("OmnidirectionalShadowMap.shader.slang", "Omnidirectional Shader");
        m_omnidirectionalMaterial = gfx->CreateMaterial("OmnidirectionalShadowMapMaterial", m_omnidirectionalShader);

        m_omnidirectionalShader->GetShaderLayout()->Print();
        m_omnidirectionalMaterial->SetProperty("LightData.data", m_omnidirectionalBuffer);
    }

    //Convert from projection space to texture space
    mat4 ToMat4(float m[16])
    {
        return mat4(m[0], m[1], m[2], m[3],
                    m[4], m[5], m[6], m[7],
                    m[8], m[9], m[10], m[11],
                    m[12], m[13], m[14], m[15]);
    }

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


        float tileSize = size/ static_cast<float>(resolution);



        for (auto& light : lights)
        {
            if (light->GetLightType() == LightType::Point)
            {
                float2 coordinates = {count % widthCount, count / widthCount};
                auto translationMatrix = translate(mat4(1.0), -light->GetTransform().GetLocalPosition());

                // translationVec.x = light->GetTransform().GetLocalPosition().x;
                // translationVec.y = light->GetTransform().GetLocalPosition().y;
                // translationVec.z = light->GetTransform().GetLocalPosition().z;
                //
                // translation.SetTranslation(-translationVec);

                float4x4 Ta, Tb, Tc, Td;
                GetTextureSpaceMatrix(coordinates, static_cast<float>(size) / static_cast<float>(resolution), Ta, Tb,
                                      Tc, Td);

                float nearplane = 0.1f;
                float farplane = 50.f;

                const float fov0 = 143.98570868f+1.99273682f;
                const float fov1 = 125.26438968f+2.78596497f;
                // float4x4 tiledShadowProjMatrices[4];
                // tiledShadowProjMatrices[0].SetPerspective(Vector2(fov0, fov1), 0.2f, farplane);
                // tiledShadowProjMatrices[1].SetPerspective(Vector2(fov1, fov0), 0.2f, farplane);
                // tiledShadowProjMatrices[2] = tiledShadowProjMatrices[0];
                // tiledShadowProjMatrices[3] = tiledShadowProjMatrices[1];
                //
                // Matrix4 shadowTexMatrices[4];

                float tilePositionX = 0;
                float tilePositionY = 0;




                // m_data.shadowMatrix[count * 4] = shadowMatrices[0];
                // m_data.shadowMatrix[count * 4 + 1] = shadowMatrices[1];
                // m_data.shadowMatrix[count * 4 + 2] = shadowMatrices[2];
                // m_data.shadowMatrix[count * 4 + 3] = shadowMatrices[3];

                count++;
            }
        }

        cmd->UploadBufferData(m_omnidirectionalBuffer.get(), reinterpret_cast<const byte*>(&m_data),
                              sizeof(OmnidirectionalShadowmapData), 0);
    }

    void OmnidirectionalShadowmapRenderPass::Render(RenderFrameData& frameData)
    {
        ASSERT(m_omnidirectionalBuffer != nullptr, "Trying to render shadowmap without buffer");

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

                    material->ApplyProperties(commands.get());

                    Pipeline* pipeline = GetPipeline({
                        .renderPass = frameData.currentRenderPass.get(),
                        .shader = material->GetShader().get(),
                        .renderer = frameData.renderer
                    });


                    //TODO: HACK TO BIND GLOBAL PROPERTIES TO GEOMETRY SHADER STAGE
                    frameData.areGlobalPropertiesBound = false;

                    BindPipeline(pipeline, frameData);

                    for (const auto& mat : model->Materials)
                    {
                        mat->ApplyProperties(commands.get());
                    }

                    BindMaterialDescriptorSets(pipeline, material, frameData);

                    commands->BindMesh(mesh.get(), material->GetShader()->GetVertexLayout());

                    auto ubo = frameData.graphics->GetUniformBufferObject(drawCall);
                    ubo.custom.x = lightCount;

                    Viewport viewport;
                    viewport.maxDepth = 1.0f;
                    viewport.minDepth = 0.0f;
                    viewport.rect = {0, 0, 4096, 4096};
                    commands->SetViewport(viewport);

                    commands->PushConstants(pipeline->vkPipelineLayoutHandle,
                                 material->GetShader()->GetShaderStages(), 0,
                                            sizeof(UniformBufferObject), &ubo);

                    commands->DrawIndexed(static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);

                    AfterRenderDrawcall(frameData, drawCall);
                }
                lightCount++;
            }
        }
    }

    void OmnidirectionalShadowmapRenderPass::SetBuffer(const SPtr<GraphicsBuffer>& buffer)
    {
        ASSERT(buffer != nullptr, "Trying to set null buffer");

        m_omnidirectionalBuffer = buffer;
    }
}
