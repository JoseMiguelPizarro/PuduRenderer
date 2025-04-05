#pragma once
#include <unordered_map>
#include "Pipeline.h"
#include "RenderFrameData.h"
#include "FrameGraph/FrameGraph.h"

namespace Pudu
{
    class RenderPass;
    class Pipeline;
    class Shader;

    class Renderer
    {
    public:
        Pipeline* GetOrCreatePipeline(PipelineQueryData query);

        Pipeline* CreatePipelineByRenderPassAndShader(RenderPass* renderPass, IShaderObject* shader);

        void AddRenderPass(RenderPass* renderPass);

        void Init(PuduGraphics* graphics, PuduApp* app);

        void Render(Scene* scene);

        virtual void SetRenderCamera(Camera* camera)
        {
            m_isRenderCameraDirty = true;
            m_renderCamera = camera;
        };
        Camera* GetRenderCamera();

        void UploadCameraData(RenderFrameData& frameData);

    protected:
        friend class FrameGraph;

        PuduGraphics* graphics;
        PuduApp* app;

        virtual void OnUploadCameraData(RenderFrameData& frameData) {};
        virtual void OnInit(PuduGraphics* graphics, PuduApp* app)
        {
        };

        virtual void OnRender(RenderFrameData& data)
        {
        };

        bool m_isRenderCameraDirty = false;
        FrameGraph frameGraph;
        FrameGraphBuilder frameGraphBuilder;
        std::unordered_map<RenderPass*, std::unordered_map<IShaderObject*, Pipeline*>> m_pipelinesByRenderPass;
        Camera* m_renderCamera;
    };
}
