#pragma once
#include <vector>
#include <DrawCall.h>

namespace Pudu {
	class Scene
	{
	public:
		std::vector<DrawCall> GetDrawCalls() {
			return m_DrawCalls;
		}

		void AddModel(Model* model);

	private:
		std::vector<DrawCall> m_DrawCalls;
	};
}

