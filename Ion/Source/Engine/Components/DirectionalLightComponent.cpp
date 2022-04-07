#include "IonPCH.h"

#include "DirectionalLightComponent.h"
#include "Engine/Entity/Entity.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(DirectionalLightComponent)

	DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

	DEFINE_NCPROPERTY(LightColor, "Light Color");
	NCPROPERTY_PARAM(LightColor, DefaultValue, Vector3(1.0f, 1.0f, 1.0f));

	DEFINE_NCPROPERTY(Intensity, "Intensity");
	NCPROPERTY_PARAM(Intensity, DefaultValue, 1.0f);
	NCPROPERTY_PARAM(Intensity, MinValue, 0.0f);

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

	DirectionalLightComponent::DirectionalLightComponent()
	{
	}
}
