#pragma once
#include <ImGui/imgui.h>
#include "Transform.h"
#include "Entity.h"
#include "ShaderCompilation/ShaderLayout.h"
#include "Shader.h"

namespace Pudu
{
    class ImGuiUtils
    {
    public:
        static void DrawTransform(Transform& transform)
        {
            ImGui::InputFloat3("Position", &transform.m_localPosition[0]);
            ImGui::InputFloat3("Scale", &transform.m_localScale[0]);
            ImGui::InputFloat3("Rotation", &transform.m_localRotation[0]);
        }

        static void DrawEntityNode(EntitySPtr entity)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
            static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

            const bool is_folder = (entity->ChildCount() > 0);
            if (is_folder)
            {
                bool open = ImGui::TreeNodeEx(entity->GetName().c_str(), tree_node_flags);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(entity->ClassName().c_str());
                if (open)
                {
                    for (auto child : entity->GetChildren())
                    {
                        DrawEntityNode(child);
                    }
                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::TreeNodeEx(entity->GetName().c_str(),
                                  tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet |
                                  ImGuiTreeNodeFlags_NoTreePushOnOpen);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(entity->ClassName().c_str());
            }
        }

        static void DrawEntityTree(std::vector<EntitySPtr> const& entities)
        {
            if (entities.size() == 0)
            {
                return;
            }

            EntitySPtr root = entities[0]->GetRoot();

            if (ImGui::TreeNode("Entities"))
            {
                static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
                    ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

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

        static void DrawShaderNode(ShaderNode* node)
        {
            if (ImGui::TreeNode(node->name.c_str()))
            {
                static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
                    ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

                ImGui::Text("Scope: %s", node->scope.c_str());

                if (ImGui::BeginTable("ShaderProperties", 2, flags))
                {
                    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 3 * 12.0f);
                    ImGui::TableHeadersRow();

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Type");
                    ImGui::TableNextColumn();
                    ImGui::Text(SHADER_NODE_TYPE_NAMES[node->type]);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("BindingIndex");
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", node->bindingIndex);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("SetIndex");
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", node->setIndex);

                    if (node->type == ShaderNodeType::CBuffer || node->type == Buffer || node->type == Uniform || node->
                        type == PushConstant)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Size");
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", node->size);


                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Offset");
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", node->offset);
                    }

                    ImGui::EndTable();
                }

                for (Size i = 0; i < node->childCount; i++)
                {
                    DrawShaderNode(node->GetChild(i));
                }

                ImGui::TreePop();
            }
        }

        static void DrawShaderTree(std::vector<SPtr<Shader>> const& shaders)
        {
            if (shaders.size() == 0)
                return;

            if (ImGui::CollapsingHeader("Shaders"))
            {
                for (const auto shader : shaders)
                {
                    if (ImGui::TreeNode(shader->GetName()))
                    {
                        auto rootNode = shader->GetShaderLayout();
                        DrawShaderNode(rootNode);
                        ImGui::TreePop();
                    }
                }
            }
        }
    };
}
