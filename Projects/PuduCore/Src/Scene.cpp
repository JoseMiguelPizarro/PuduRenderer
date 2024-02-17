#include "Scene.h"
#include <ImGui/imgui.h>
#include "ImGuiUtils.h"

namespace Pudu {
	void Scene::AddEntity(EntitySPtr entity)
	{
		m_entities.push_back(entity);
		if (entity != sceneRoot && entity->GetParent() == nullptr)
		{
			entity->SetParent(sceneRoot);
		}

		entity->AttatchToScene(*this);
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
		//TODO
	}

	void Scene::AddRendererEntity(RenderEntitySPtr renderEntity)
	{
		m_renderEntities.push_back(renderEntity);

		auto& model = renderEntity->GetModel();
		for (size_t i = 0; i < model.Meshes.size(); i++)
		{
			Material material = model.Materials[0];
			if (i >= model.Materials.size())
			{
				material = model.Materials[0];
			}

			Model m = model;
			model.Transform = renderEntity->GetTransform();
			DrawCall dc(m, model.Meshes[i], material);
			dc.TransformMatrix = renderEntity->GetTransform().GetTransformationMatrix();
			m_DrawCalls.push_back(dc);
		}
	}
	void Scene::RemoveRenderEntity(RenderEntitySPtr renderEntity)
	{
		//TODO
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
	std::vector<RenderEntitySPtr> Scene::GetRenderEntities()
	{
		return m_renderEntities;
	}
}
