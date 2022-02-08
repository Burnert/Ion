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

	class ION_API DirectionalLightComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY();

		// Component Callback methods

		void COMPCALLBACKFUNC OnCreate();
		void COMPCALLBACKFUNC OnDestroy();
		void COMPCALLBACKFUNC BuildRendererData(RRendererData& data);

		// End of Component Callback methods

		DirectionalLightComponentData& GetDirectionalLightDataRef();

	private:
		DirectionalLightComponent();

	private:
		DirectionalLightComponentData m_DirectionalLightData;
	};

	template<>
	struct ComponentTypeDefaults<DirectionalLightComponent>
	{
		static constexpr const char* Name = "Directional Light Component";
	};
}
