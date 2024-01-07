#pragma once
#include <ImGui/imgui.h>
#include "Transform.h"

class ImGuiUtils {
public:
	static void DrawTransform(Transform& transform) {
		ImGui::InputFloat3("Position", &transform.Position[0]);
		ImGui::InputFloat3("Scale", &transform.Scale[0]);
		ImGui::InputFloat3("Rotation", &transform.Rotation[0]);
	}
};