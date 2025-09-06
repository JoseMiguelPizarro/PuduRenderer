#include <glm/fwd.hpp>
#include "Lighting/Light.h"
#include <PuduMath.h>

namespace Pudu
{


	float Light::GetIlluminance()
	{
		return intensity / (range * range);
	}
}

