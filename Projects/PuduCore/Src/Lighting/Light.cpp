#include "Lighting/Light.h"

namespace Pudu
{
	float Light::GetIlluminance()
	{
		return intensity / (range * range);
	}
}

