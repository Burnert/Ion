#pragma once

#include "SceneComponent.h"
#include "Renderer/Light.h"

namespace Ion
{
	struct DirectionalLightComponentData
	{
		Vector3 LightColor;
		float Intensity;
	};

	ENTITY_COMPONENT_CLASS_HEADER(DirectionalLightComponent);

	class ION_API DirectionalLightComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(DirectionalLightComponent, "Directional Light");

		void SERIALCALL BuildRendererData(RRendererData& data);

		DECLARE_NCPROPERTY(Vector3, LightColor)
		DECLARE_NCPROPERTY(float,   Intensity)

		DirectionalLightComponentData& GetDirectionalLightDataRef();

	private:
		DirectionalLightComponent();

	private:
		DirectionalLightComponentData m_DirectionalLightData;
	};
}
