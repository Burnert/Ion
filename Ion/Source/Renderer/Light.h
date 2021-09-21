#pragma once

#include "Core/Core.h"

namespace Ion
{
	class ION_API Light
	{
	public:
		Vector3 m_Location;
		float m_Intensity;
		Vector3 m_Color;
	};

	class ION_API DirectionalLight : public Light
	{
	public:
		Vector3 m_LightDirection;
	};
}
