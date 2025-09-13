//
// Created by Hojaverde on 9/12/2025.
//

#include "OmnidirectionalShadowmapRenderPass.h"

#include "PuduGraphics.h"
#include "RenderFrameData.h"

namespace Pudu
{
void OmnidirectionalShadowmapRenderPass::OnCreate(PuduGraphics* gfx)
{
    m_omnidirectionalBuffer = gfx->CreateGraphicsBuffer(sizeof(OmnidirectionalShadowmapData),nullptr,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                    |VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT, "OmnidirectionalLightBuffer");

    m_omnidirectionalShader = gfx->CreateShader("OmnidirectionalShadowMap.shader.slang","Omnidirectional Shader");
}
//Convert from projection space to texture space
void GetTextureSpaceMatrix(float2 p,float size, float4x4& Ma, float4x4& Mb,float4x4& Mc,float4x4& Md)
{
    float s = size; //Size
    Ma = float4x4({s,0,0,0},{0,s/2,0,0},{0,0,1,0},{p.x, p.y - s/2,0 ,1});
    Mb = float4x4({s/2,0,0,0},{0,s,0,0},{0,0,1,0},{p.x + s/2, p.y,0 ,1});
    Mc = float4x4({s,0,0,0},{0,s/2,0,0},{0,0,1,0},{p.x, p.y + s/2,0 ,1});
    Md = float4x4({s/2,0,0,0},{0,s,0,0},{0,0,1,0},{p.x - s/2, p.y,0 ,1});
}

void OmnidirectionalShadowmapRenderPass::PreRender(RenderFrameData& renderData)
{
    auto cmd = renderData.currentCommand;

    std::vector<SPtr<LightEntity>> lights;
    renderData.scene->GetEntitiesOfType<LightEntity>(lights);

    int count = 0;
    uint size = 256;

    uint resolution = 4096;
    uint widthCount =  resolution/size;

    for (auto& light : lights)
    {
        if (light->GetLightType() == LightType::Point)
        {
            float2 coordinates = {count%widthCount,count/widthCount};
            float4x4 Ta,Tb,Tc,Td;
            GetTextureSpaceMatrix(coordinates,size,Ta,Tb,Tc,Td);

            mat4 mA = Ta * PerspectiveMatrix(125.26438968, 143.98570868/125.26438968,0.1,100) * translate(mat4(1.0), light->GetTransform().GetLocalPosition())* toMat4( quat(float3( radians(27.36780516f),radians(180.f),0)));
            mat4 mB = Tb * PerspectiveMatrix(143.98570868, 125.26438968/143.98570868,0.1,100) * translate(mat4(1.0), light->GetTransform().GetLocalPosition())* toMat4( quat(float3( radians(27.36780516f),radians(0.f),radians(90.f))));
            mat4 mC = Tc * PerspectiveMatrix(125.26438968, 143.98570868/125.26438968,0.1,100) * translate(mat4(1.0), light->GetTransform().GetLocalPosition())* toMat4( quat(float3( radians(-27.36780516f),radians(270.f),0)));
            mat4 mD = Td * PerspectiveMatrix(143.98570868, 125.26438968/143.98570868,0.1,100) * translate(mat4(1.0), light->GetTransform().GetLocalPosition())* toMat4( quat(float3( radians(-27.36780516f),radians(90.f),radians(90.f))));

            m_data.shadowMatrix[count*4] = mA;
            m_data.shadowMatrix[count*4+1] = mB;
            m_data.shadowMatrix[count*4+2] = mC;
            m_data.shadowMatrix[count*4+3] = mD;
        }

        count++;
    }

    cmd->UploadBufferData(m_omnidirectionalBuffer.get(),reinterpret_cast<const byte*>(&m_data),sizeof(OmnidirectionalShadowmapData),0);
}

}

