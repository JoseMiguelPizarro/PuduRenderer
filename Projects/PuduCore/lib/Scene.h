#pragma once
#include <vector>
#include "DrawCall.h"
#include "PuduTime.h"
#include "Camera.h"
#include "EntityFwd.h"
#include "RenderEntity.h"
#include "EntityManager.h"
#include "PuduApp.h"
#include "Resources/Resources.h"
#include "Lighting/Light.h"
#include "RenderSettings.h"

namespace Pudu
{
    class Scene
    {
    public:
        Camera* camera = nullptr;

        void AddEntity(EntityHandle entityPtr);
        void AddEntities(EntityHandle entities);
        void RemoveEntity(EntityHandle entity);
        void RemoveRenderEntity(EntityHandle renderEntity);
        void DrawImGui();
        std::vector<EntityHandle> GetEntities();
        std::vector<RenderEntitySPtr> GetRenderEntities();
        ResourceHandle<EntitySPtr> sceneRoot;
        PuduTime* time;
        Light* directionalLight;


        const std::vector<DrawCall>* GetDrawCalls(uint32_t layer) const
        {
            return &m_drawCallsPerLayer[layer];
        }

        Scene(const PuduApp* app)
        {
            time = app->Time.get();
            sceneRoot = app->EntityManager->AllocateEntity()->GetHandle();
            sceneRoot->SetName("SceneRoot");
            AddEntity(sceneRoot);
        }

        PuduTime* Time = nullptr;

    private:
        friend class Entity;
        friend class RenderEntity;

        void AddRendererEntity(ResourceHandle<RenderEntitySPtr> renderEntity);
        void AddDrawCall(DrawCall& drawCall, RenderSettings& settings);
        std::vector<ResourceHandle<EntitySPtr>> m_entities;
        std::vector<ResourceHandle<RenderEntitySPtr>> m_renderEntities;

        std::vector<DrawCall> m_drawCallsPerLayer[32];
    };
}
