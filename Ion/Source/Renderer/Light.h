#pragma once

#include "Core/Core.h"

namespace Ion
{
	enum class ELightType : uint8
	{
		Point = 1,
		Directional = 2,
	};

	struct RLightRenderProxy
	{
		Vector3 Location;
		Vector3 Color;
		Vector3 Direction;
		float Intensity;
		float Falloff;
		ELightType Type;
	};

	class ION_API Light
	{
		friend class Renderer;
#if PLATFORM_SUPPORTS_OPENGL
		friend class OpenGLRenderer;
#endif
	public:
		void CopyRenderData(RLightRenderProxy& outRenderProxy)
		{
			outRenderProxy.Location = m_Location;
			outRenderProxy.Color = m_Color;
			outRenderProxy.Intensity = m_Intensity;
			outRenderProxy.Falloff = m_Falloff;
			outRenderProxy.Type = ELightType::Point;
		}

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
		void CopyRenderData(RLightRenderProxy& outRenderProxy)
		{
			outRenderProxy.Location = m_Location;
			outRenderProxy.Color = m_Color;
			outRenderProxy.Direction = m_LightDirection;
			outRenderProxy.Intensity = m_Intensity;
			outRenderProxy.Falloff = m_Falloff;
			outRenderProxy.Type = ELightType::Directional;
		}

		Vector3 m_LightDirection;
	};
}
