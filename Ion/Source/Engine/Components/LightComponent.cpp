#include "IonPCH.h"

#include "LightComponent.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(LightComponent)

	ENTITY_COMPONENT_STATIC_CALLBACK_ONCREATE_FUNC()
	ENTITY_COMPONENT_STATIC_CALLBACK_ONDESTROY_FUNC()
	ENTITY_COMPONENT_STATIC_CALLBACK_BUILDRENDERERDATA_FUNC()

	void LightComponent::OnCreate()
	{
	}

	void LightComponent::OnDestroy()
	{
	}

	void LightComponent::SetTransform(const Transform& transform)
	{
		m_SceneData.Transform = transform;
	}

	const Transform& LightComponent::GetTransform() const
	{
		return m_SceneData.Transform;
	}

	void LightComponent::SetLocation(const Vector3& location)
	{
		m_SceneData.Transform.SetLocation(location);
	}

	const Vector3& LightComponent::GetLocation() const
	{
		return m_SceneData.Transform.GetLocation();
	}

	void LightComponent::SetRotation(const Rotator& rotation)
	{
		m_SceneData.Transform.SetRotation(rotation);
	}

	const Rotator& LightComponent::GetRotation() const
	{
		return m_SceneData.Transform.GetRotation();
	}

	void LightComponent::SetScale(const Vector3& scale)
	{
		m_SceneData.Transform.SetScale(scale);
	}

	const Vector3& LightComponent::GetScale() const
	{
		return m_SceneData.Transform.GetScale();
	}

	void LightComponent::SetVisible(bool bVisible)
	{
		m_SceneData.bVisible = bVisible;
	}

	bool LightComponent::IsVisible() const
	{
		return m_SceneData.bVisible;
	}

	void LightComponent::SetVisibleInGame(bool bVisibleInGame)
	{
		m_SceneData.bVisibleInGame = bVisibleInGame;
	}

	bool LightComponent::IsVisibleInGame() const
	{
		return m_SceneData.bVisibleInGame;
	}

	LightComponentData& LightComponent::GetLightDataRef()
	{
		return m_LightData;
	}

	void LightComponent::BuildRendererData(RRendererData& data)
	{
		RLightRenderProxy light { };
		light.Location  = m_SceneData.Transform.GetLocation();
		light.Color     = m_LightData.LightColor;
		light.Intensity = m_LightData.Intensity;
		light.Falloff   = m_LightData.Falloff;
		light.Type      = ELightType::Point;
		data.AddLight(light);
	}

	LightComponent::LightComponent() :
		m_SceneData({ }),
		m_LightData({ })
	{
		SetTickEnabled(false);
		InitAsSceneComponent();
	}
}
