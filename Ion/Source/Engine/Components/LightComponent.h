#pragma once

#include "SceneComponent.h"
#include "Renderer/Light.h"

namespace Ion
{
	struct LightComponentData
	{
		Vector3 LightColor;
		float Intensity;
		float Falloff;
	};

	ENTITY_COMPONENT_CLASS_HEADER(LightComponent)

	class ION_API LightComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(LightComponent, "Light")

		void SERIALCALL BuildRendererData(RRendererData& data);

		DECLARE_NCPROPERTY(Vector3, LightColor)
		DECLARE_NCPROPERTY(float,   Intensity)
		DECLARE_NCPROPERTY(float,   Falloff)

		LightComponentData& GetLightDataRef();

	private:
		LightComponent();

	private:
		LightComponentData m_LightData;
	};
}
