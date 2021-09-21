#pragma once

#include "Core/Core.h"

namespace Ion
{
	class ION_API Light
	{
		friend class Renderer;
#if PLATFORM_SUPPORTS_OPENGL
		friend class OpenGLRenderer;
#endif
	public:
		Vector3 m_Location;
		float m_Intensity;
		float m_Falloff;
		Vector3 m_Color;
	};

	class ION_API DirectionalLight : public Light
	{
		friend class Renderer;
#if PLATFORM_SUPPORTS_OPENGL
		friend class OpenGLRenderer;
#endif
	public:
		Vector3 m_LightDirection;
	};
}
