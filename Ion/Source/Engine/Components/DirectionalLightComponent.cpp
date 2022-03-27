#include "IonPCH.h"

#include "DirectionalLightComponent.h"
#include "Engine/Entity.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(DirectionalLightComponent)

	DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

	DirectionalLightComponentData& DirectionalLightComponent::GetDirectionalLightDataRef()
	{
		return m_DirectionalLightData;
	}

	void DirectionalLightComponent::BuildRendererData(RRendererData& data)
	{
		if (!IsVisible() || !GetOwner()->IsVisible())
			return;

		// @TODO: If active dirlight:
		Transform worldTransform = GetWorldTransform();

		RLightRenderProxy light { };
		light.Color     = m_DirectionalLightData.LightColor;
		light.Intensity = m_DirectionalLightData.Intensity;
		light.Direction = worldTransform.GetForwardVector();
		light.Type      = ELightType::Directional;
		data.DirectionalLight = light;
	}

	DirectionalLightComponent::DirectionalLightComponent() :
		m_DirectionalLightData({ })
	{
	}
}
