#include "IonPCH.h"

#include "LightComponent.h"
#include "Engine/Entity/EntityOld.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(LightComponent)

	DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

	DEFINE_NCPROPERTY(LightColor, "Light Color");
	NCPROPERTY_PARAM(LightColor, DefaultValue, Vector3(1.0f, 1.0f, 1.0f));

	DEFINE_NCPROPERTY(Intensity, "Intensity");
	NCPROPERTY_PARAM(Intensity, DefaultValue, 1.0f);
	NCPROPERTY_PARAM(Intensity, MinValue, 0.0f);

	DEFINE_NCPROPERTY(Falloff, "Falloff");
	NCPROPERTY_PARAM(Falloff, DefaultValue, 4.0f);
	NCPROPERTY_PARAM(Falloff, MinValue, 0.0f);

	void LightComponent::BuildRendererData(RRendererData& data)
	{
		if (ShouldBeRendered())
		{
			Transform worldTransform = GetWorldTransform();

			RLightRenderProxy light { };
			light.Location = worldTransform.GetLocation();
			light.Color = LightColor;
			light.Intensity = Intensity;
			light.Falloff = Falloff;
			light.Type = ELightType::Point;
			data.AddLight(light);
		}
	}

	LightComponent::LightComponent()
	{
	}
}
