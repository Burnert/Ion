#include "IonPCH.h"

#include "LightComponent.h"
#include "Engine/Entity/Entity.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(LightComponent)

	//DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

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
			light.Color = m_LightData.LightColor;
			light.Intensity = m_LightData.Intensity;
			light.Falloff = m_LightData.Falloff;
			light.Type = ELightType::Point;
			data.AddLight(light);
		}
	}

	LightComponent::LightComponent() :
		m_LightData({ })
	{
	}
}
