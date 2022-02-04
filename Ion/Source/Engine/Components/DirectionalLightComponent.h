#pragma once

#include "Component.h"
#include "Renderer/Light.h"

namespace Ion
{
	struct DirectionalLightComponentData
	{
		Vector3 LightColor;
		float Intensity;
	};

	class ION_API DirectionalLightComponent : public Component
	{
		ENTITY_COMPONENT_CLASS_BODY();

		// Component Callback methods

		void COMPCALLBACKFUNC OnCreate();
		void COMPCALLBACKFUNC OnDestroy();
		void COMPCALLBACKFUNC BuildRendererData(RRendererData& data);

		// End of Component Callback methods

		void SetTransform(const Transform& transform);
		const Transform& GetTransform() const;

		void SetLocation(const Vector3& location);
		const Vector3& GetLocation() const;

		void SetRotation(const Rotator& rotation);
		const Rotator& GetRotation() const;

		void SetScale(const Vector3& scale);
		const Vector3& GetScale() const;

		void SetVisible(bool bVisible);
		bool IsVisible() const;

		void SetVisibleInGame(bool bVisibleInGame);
		bool IsVisibleInGame() const;

		DirectionalLightComponentData& GetDirectionalLightDataRef();

	private:
		DirectionalLightComponent();

	private:
		SceneComponentData m_SceneData;
		DirectionalLightComponentData m_DirectionalLightData;
	};
}