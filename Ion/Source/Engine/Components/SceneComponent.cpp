#include "IonPCH.h"

#include "SceneComponent.h"
#include "Engine/Entity.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(EmptySceneComponent)

	void SceneComponent::SetTransform(const Transform& transform)
	{
		m_SceneData.RelativeTransform = transform;
		UpdateWorldTransformCache();
	}

	void SceneComponent::SetLocation(const Vector3& location)
	{
		m_SceneData.RelativeTransform.SetLocation(location);
		UpdateWorldTransformCache();
	}

	void SceneComponent::SetRotation(const Rotator& rotation)
	{
		m_SceneData.RelativeTransform.SetRotation(rotation);
		UpdateWorldTransformCache();
	}

	void SceneComponent::SetScale(const Vector3& scale)
	{
		m_SceneData.RelativeTransform.SetScale(scale);
		UpdateWorldTransformCache();
	}

	void SceneComponent::SetVisible(bool bVisible)
	{
		m_SceneData.bVisible = bVisible;
	}

	void SceneComponent::SetVisibleInGame(bool bVisibleInGame)
	{
		m_SceneData.bVisibleInGame = bVisibleInGame;
	}

	bool SceneComponent::ShouldBeRendered() const
	{
		ionassert(GetOwner());
		return IsVisible() && GetOwner()->IsVisible();
	}

	void SceneComponent::AttachTo(SceneComponent* parent)
	{
		if (!parent)
		{
			Detach();
		}

		// Only bind the component if it's not owned yet.
		if (!GetOwner())
		{
			parent->GetOwner()->BindComponent(this);
		}

		ionassert(parent->GetOwner() == GetOwner(),
			"Cannot attach Scene Component to one with a different owning Entity.");

		// Remove from current parent first
		if (m_Parent)
		{
			m_Parent->RemoveChild(this);
		}

		m_Parent = parent;
		m_Parent->AddChild(this);
	}

	SceneComponent* SceneComponent::Detach()
	{
		if (m_Parent)
		{
			m_Parent->RemoveChild(this);
			m_Parent = nullptr;

			GetOwner()->UnbindComponent(this);
		}
		return this;
	}

	TArray<SceneComponent*> SceneComponent::GetAllDescendants() const
	{
		TArray<SceneComponent*> descendants;
		GetAllDescendants_Internal(descendants);
		return descendants;
	}

	void SceneComponent::AddChild(SceneComponent* component)
	{
		TRACE_FUNCTION();

		ionassert(component);
		ionassert(std::find(m_Children.begin(), m_Children.end(), component) == m_Children.end());

		m_Children.push_back(component);
	}

	void SceneComponent::RemoveChild(SceneComponent* component)
	{
		TRACE_FUNCTION();

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

	void SceneComponent::UpdateWorldTransformCache()
	{
		if (m_Parent)
		{
			m_SceneData.WorldTransformCache = m_SceneData.RelativeTransform * m_Parent->GetWorldTransform();
		}
		else
		{
			ionassert(GetOwner());
			m_SceneData.WorldTransformCache = m_SceneData.RelativeTransform * GetOwner()->GetWorldTransform();
		}

		UpdateChildrenWorldTransformCache();
	}

	void SceneComponent::UpdateChildrenWorldTransformCache()
	{
		for (SceneComponent* child : m_Children)
		{
			child->UpdateWorldTransformCache();
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
