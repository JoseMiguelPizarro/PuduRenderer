#pragma once
#include <vector>
#include "DrawCall.h"
#include "PuduTime.h"
#include "Camera.h";

namespace Pudu {
	class Scene
	{
	public:
		std::vector<DrawCall> GetDrawCalls() {
			return m_DrawCalls;
		}

		Camera* Camera = nullptr;

		Scene() {}
		Scene(PuduTime* time) {
			Time = time;
		}

		PuduTime* Time = nullptr;
		void AddModel(Model* model);

	private:
		std::vector<DrawCall> m_DrawCalls;
	};
}

