#pragma once
#include <PuduMath.h>
namespace Pudu
{
	struct Viewport
	{
		Rect2DInt                       rect;
		float                             minDepth = 0.0f;
		float                             maxDepth = 0.0f;
	};
}
