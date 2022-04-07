#include "IonPCH.h"

#include "LightComponent.h"
#include "Engine/Entity/Entity.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(LightComponent)

	DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

	DEFINE_NCPROPERTY(LightColor, "Light Color", [] { })
	DEFINE_NCPROPERTY(Intensity,  "Intensity",   [] { })
	DEFINE_NCPROPERTY(Falloff,    "Falloff",     [] { })

	LightComponentData& LightComponent::GetLightDataRef()
	{
		return m_LightData;
	}

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

	LightComponent::LightComponent() :
		m_LightData({ }),
		LightColor(Vector3(1.0f, 1.0f, 1.0f)),
		Intensity(1.0f),
		Falloff(4.0f)
	{
	}
}
