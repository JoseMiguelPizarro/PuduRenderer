#pragma once
#include <ImGui/imgui.h>
#include "Transform.h"

class ImGuiUtils {
public:
	static void DrawTransform(Transform& transform) {
		ImGui::InputFloat3("Position", &transform.LocalPosition[0]);
		ImGui::InputFloat3("Scale", &transform.LocalScale[0]);
		ImGui::InputFloat3("Rotation", &transform.LocalRotation[0]);
	}
};