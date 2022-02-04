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

	void DirectionalLightComponent::SetTransform(const Transform& transform)
	{
		m_SceneData.Transform = transform;
	}

	const Transform& DirectionalLightComponent::GetTransform() const
	{
		return m_SceneData.Transform;
	}

	void DirectionalLightComponent::SetLocation(const Vector3& location)
	{
		m_SceneData.Transform.SetLocation(location);
	}

	const Vector3& DirectionalLightComponent::GetLocation() const
	{
		return m_SceneData.Transform.GetLocation();
	}

	void DirectionalLightComponent::SetRotation(const Rotator& rotation)
	{
		m_SceneData.Transform.SetRotation(rotation);
	}

	const Rotator& DirectionalLightComponent::GetRotation() const
	{
		return m_SceneData.Transform.GetRotation();
	}

	void DirectionalLightComponent::SetScale(const Vector3& scale)
	{
		m_SceneData.Transform.SetScale(scale);
	}

	const Vector3& DirectionalLightComponent::GetScale() const
	{
		return m_SceneData.Transform.GetScale();
	}

	void DirectionalLightComponent::SetVisible(bool bVisible)
	{
		m_SceneData.bVisible = bVisible;
	}

	bool DirectionalLightComponent::IsVisible() const
	{
		return m_SceneData.bVisible;
	}

	void DirectionalLightComponent::SetVisibleInGame(bool bVisibleInGame)
	{
		m_SceneData.bVisibleInGame = bVisibleInGame;
	}

	bool DirectionalLightComponent::IsVisibleInGame() const
	{
		return m_SceneData.bVisibleInGame;
	}

	DirectionalLightComponentData& DirectionalLightComponent::GetDirectionalLightDataRef()
	{
		return m_DirectionalLightData;
	}

	void DirectionalLightComponent::BuildRendererData(RRendererData& data)
	{
		// @TODO: If active dirlight:

		RLightRenderProxy light { };
		light.Color     = m_DirectionalLightData.LightColor;
		light.Intensity = m_DirectionalLightData.Intensity;
		light.Direction = m_SceneData.Transform.GetForwardVector();
		light.Type      = ELightType::Directional;
		data.DirectionalLight = light;
	}

	DirectionalLightComponent::DirectionalLightComponent() :
		m_SceneData({ }),
		m_DirectionalLightData({ })
	{
		SetTickEnabled(false);
		InitAsSceneComponent();
	}
}
