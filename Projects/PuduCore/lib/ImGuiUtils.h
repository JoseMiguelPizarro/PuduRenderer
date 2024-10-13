#pragma once
#include <ImGui/imgui.h>
#include "Transform.h"
#include "Entity.h"

namespace Pudu {
	class ImGuiUtils {
	public:
		static void DrawTransform(Transform& transform) {
			ImGui::InputFloat3("Position", &transform.m_localPosition[0]);
			ImGui::InputFloat3("Scale", &transform.m_localScale[0]);
			ImGui::InputFloat3("Rotation", &transform.m_localRotation[0]);
		}

		static void DrawEntityNode(EntitySPtr entity) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
			static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

			const bool is_folder = (entity->ChildCount() > 0);
			if (is_folder)
			{
				bool open = ImGui::TreeNodeEx(entity->GetName().c_str(), tree_node_flags);
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(entity->ClassName().c_str());
				if (open)
				{
					for (auto child : entity->GetChildren()) {
						DrawEntityNode(child);
					}
					ImGui::TreePop();
				}
			}
			else
			{
				ImGui::TreeNodeEx(entity->GetName().c_str(), tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(entity->ClassName().c_str());
			}
		}

		static void DrawEntityTree(std::vector<EntitySPtr>const& entities)
		{
			if (entities.size() == 0)
			{
				return;
			}

			EntitySPtr root = entities[0]->GetRoot();

			if (ImGui::TreeNode("Entities"))
			{
				static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

				static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

				if (ImGui::BeginTable("entities", 2, flags))
				{
					// The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 4 * 18.0f);
					ImGui::TableHeadersRow();

					DrawEntityNode(root);

					ImGui::EndTable();
				}
				ImGui::TreePop();
			}
		}
	};
}