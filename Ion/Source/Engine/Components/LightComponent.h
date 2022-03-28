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

	class ION_API LightComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(LightComponent, "Light")

		// Component Callback methods

		void SERIALCALL BuildRendererData(RRendererData& data);

		// End of Component Callback methods

		LightComponentData& GetLightDataRef();

	private:
		LightComponent();

	private:
		LightComponentData m_LightData;
	};
}
