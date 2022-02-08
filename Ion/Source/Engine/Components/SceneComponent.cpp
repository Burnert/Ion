#include "IonPCH.h"

#include "SceneComponent.h"
#include "Engine/Entity.h"

namespace Ion
{
	void SceneComponent::SetTransform(const Transform& transform)
	{
		m_SceneData.Transform = transform;
	}

	const Transform& SceneComponent::GetTransform() const
	{
		return m_SceneData.Transform;
	}

	void SceneComponent::SetLocation(const Vector3& location)
	{
		m_SceneData.Transform.SetLocation(location);
	}

	const Vector3& SceneComponent::GetLocation() const
	{
		return m_SceneData.Transform.GetLocation();
	}

	void SceneComponent::SetRotation(const Rotator& rotation)
	{
		m_SceneData.Transform.SetRotation(rotation);
	}

	const Rotator& SceneComponent::GetRotation() const
	{
		return m_SceneData.Transform.GetRotation();
	}

	void SceneComponent::SetScale(const Vector3& scale)
	{
		m_SceneData.Transform.SetScale(scale);
	}

	const Vector3& SceneComponent::GetScale() const
	{
		return m_SceneData.Transform.GetScale();
	}

	void SceneComponent::SetVisible(bool bVisible)
	{
		m_SceneData.bVisible = bVisible;
	}

	bool SceneComponent::IsVisible() const
	{
		return m_SceneData.bVisible;
	}

	void SceneComponent::SetVisibleInGame(bool bVisibleInGame)
	{
		m_SceneData.bVisibleInGame = bVisibleInGame;
	}

	bool SceneComponent::IsVisibleInGame() const
	{
		return m_SceneData.bVisibleInGame;
	}

	Transform SceneComponent::GetWorldTransform() const
	{
		Transform worldTransform = GetTransform();
		if (GetOwner())
		{
			worldTransform *= GetOwner()->GetTransform();
		}
		return worldTransform;
	}

	SceneComponent::SceneComponent() :
		m_SceneData({ })
	{
		InitAsSceneComponent();
	}
}
