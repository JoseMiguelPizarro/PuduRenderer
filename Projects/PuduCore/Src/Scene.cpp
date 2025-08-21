#include "Scene.h"
#include <ImGui/imgui.h>
#include "ImGuiUtils.h"

namespace Pudu
{
    void Scene::AddEntity(const EntitySPtr& entity)
    {
        m_entities.push_back(entity);
        if (entity != sceneRoot && entity->GetParent() == nullptr)
        {
            entity->SetParent(sceneRoot);
        }

        entity->AttatchToScene(*this);

        for (const auto& child : entity->GetChildren())
        {
            AddEntity(child);
        }
    }

    void Scene::AddEntities(std::vector<EntitySPtr> entities)
    {
        for (auto e : entities)
        {
            AddEntity(e);
        }
    }

    void Scene::RemoveEntity(EntitySPtr entity)
    {
        RemoveEntityInternal(entity, true);
    }

    void Scene::RemoveEntityInternal(EntitySPtr entity, bool isRoot)
    {
        m_entities.erase
        (
            std::ranges::remove_if
            (m_entities,
             [entity](const auto& ps) { return ps == entity; }
            ).begin(), m_entities.end()
        );

        if (isRoot)
        {
            entity->SetParent(nullptr);
        }

        for (auto& child : entity->GetChildren())
        {
            RemoveEntityInternal(child, false);
        }
    }

    void Scene::DrawImGui()
    {
        ImGui::NewFrame();
        ImGui::Begin("Pudu Renderer Debug");
        ImGui::Text("Camera:");

        vec3 cameraFwd = camera->Transform.GetForward();

        ImGuiUtils::DrawTransform(camera->Transform);

        ImGui::Text(std::format("Cam Forward: {},{},{}", cameraFwd.x, cameraFwd.y, cameraFwd.z).c_str());
        ImGui::Text(std::format("FPS: {}", Time->GetFPS()).c_str());
        ImGui::Text(std::format("Delta Time: {}", time->DeltaTime()).c_str());

        auto entities = GetEntities();

        //Tree begin
        ImGuiUtils::DrawEntityTree(entities);

        //Tree end
        ImGui::End();
        ImGui::Render();
    }

    std::vector<EntitySPtr> Scene::GetEntities()
    {
        return m_entities;
    }
}
