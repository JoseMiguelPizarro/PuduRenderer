#pragma once

#include "Model.h"
namespace Pudu
{
	class DrawCall
	{
	public:
		Model* ModelPtr;

		DrawCall() {}
		DrawCall(Model* model)
		{
			ModelPtr = model;
		}
	};
}
