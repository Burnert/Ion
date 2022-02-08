#include "IonPCH.h"

#include "DirectionalLightComponent.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(DirectionalLightComponent)

	ENTITY_COMPONENT_STATIC_CALLBACK_ONCREATE_FUNC()
	ENTITY_COMPONENT_STATIC_CALLBACK_ONDESTROY_FUNC()
	ENTITY_COMPONENT_STATIC_CALLBACK_BUILDRENDERERDATA_FUNC()

	void DirectionalLightComponent::OnCreate()
	{
	}

	void DirectionalLightComponent::OnDestroy()
	{
	}

	DirectionalLightComponentData& DirectionalLightComponent::GetDirectionalLightDataRef()
	{
		return m_DirectionalLightData;
	}

	void DirectionalLightComponent::BuildRendererData(RRendererData& data)
	{
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
