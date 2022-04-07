#include "IonPCH.h"

#include "DirectionalLightComponent.h"
#include "Engine/Entity/Entity.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(DirectionalLightComponent)

	DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

	DEFINE_NCPROPERTY(LightColor, "Light Color", [] { })
	DEFINE_NCPROPERTY(Intensity,  "Intensity",   [] { })

	DirectionalLightComponentData& DirectionalLightComponent::GetDirectionalLightDataRef()
	{
		return m_DirectionalLightData;
	}

	void DirectionalLightComponent::BuildRendererData(RRendererData& data)
	{
		if (ShouldBeRendered())
		{
			// @TODO: If active dirlight:
			Transform worldTransform = GetWorldTransform();

			RLightRenderProxy light { };
			light.Color = LightColor;
			light.Intensity = Intensity;
			light.Direction = worldTransform.GetForwardVector();
			light.Type = ELightType::Directional;
			data.DirectionalLight = light;
		}
	}

	DirectionalLightComponent::DirectionalLightComponent() :
		m_DirectionalLightData({ }),
		LightColor(Vector3(1.0f, 1.0f, 1.0f)),
		Intensity(1.0f)
	{
	}
}
