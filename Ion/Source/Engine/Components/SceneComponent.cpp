#include "IonPCH.h"

#include "SceneComponent.h"
#include "Engine/Entity.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(EmptySceneComponent)

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

		Entity* owner = GetOwner();
		if (owner)
		{
			worldTransform *= owner->GetWorldTransform();
		}

		return worldTransform;
	}

	void SceneComponent::AttachTo(SceneComponent* parent)
	{
		ionassert(parent);
		ionassert(parent->GetOwner() == GetOwner(),
			"Cannot attach Scene Component to one with a different owning Entity.");

		//m_Parent = parent;
		//m_Parent->AddChild(this);

		//Entity* owner = parent->GetOwner();
	}

	void SceneComponent::Detach()
	{
		if (m_Parent)
		{
			//m_Parent->RemoveChild(this);
			//m_Parent = nullptr;
		}
	}

	TArray<SceneComponent*> SceneComponent::GetAllDescendants() const
	{
		TArray<SceneComponent*> descendants;
		GetAllDescendants_Internal(descendants);
		return descendants;
	}

	void SceneComponent::AddChild(SceneComponent* component)
	{
		ionassert(component);
		ionassert(std::find(m_Children.begin(), m_Children.end(), component) == m_Children.end());

		m_Children.push_back(component);
	}

	void SceneComponent::RemoveChild(SceneComponent* component)
	{
		ionassert(component);

		auto it = std::find(m_Children.begin(), m_Children.end(), component);
		if (it != m_Children.end())
			m_Children.erase(it);
	}

	void SceneComponent::GetAllDescendants_Internal(TArray<SceneComponent*>& outArray) const
	{
		for (SceneComponent* child : m_Children)
		{
			outArray.push_back(child);
			child->GetAllDescendants_Internal(outArray);
		}
	}

	SceneComponent::SceneComponent() :
		m_SceneData({ }),
		m_Parent(nullptr)
	{
		InitAsSceneComponent();
	}

	EmptySceneComponent::EmptySceneComponent()
	{
		SetTickEnabled(false);
	}
}
