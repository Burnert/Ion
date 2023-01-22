#include "IonPCH.h"

#include "SceneComponent.h"
#include "Engine/Entity/EntityOld.h"

namespace Ion
{
#pragma region Old Scene Component

	DECLARE_ENTITY_COMPONENT_CLASS(EmptySceneComponent)

	void SceneComponent::SetTransform(const Transform& transform)
	{
		m_RelativeTransform = transform;

		UpdateWorldTransformCache();
	}

	void SceneComponent::SetLocation(const Vector3& location)
	{
		m_RelativeTransform.SetLocation(location);

		UpdateWorldTransformCache();
	}

	void SceneComponent::SetRotation(const Rotator& rotation)
	{
		m_RelativeTransform.SetRotation(rotation);

		UpdateWorldTransformCache();
	}

	void SceneComponent::SetScale(const Vector3& scale)
	{
		m_RelativeTransform.SetScale(scale);

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

		UpdateWorldTransformCache();
	}

	SceneComponent* SceneComponent::Detach()
	{
		ionassert(GetOwner());

		if (m_Parent)
		{
			m_Parent->RemoveChild(this);
			m_Parent = nullptr;
		}
		GetOwner()->UnbindComponent(this);

		UpdateWorldTransformCache();

		return this;
	}

	TArray<SceneComponent*> SceneComponent::GetAllDescendants() const
	{
		TArray<SceneComponent*> descendants;
		GetAllDescendants_Internal(descendants);
		return descendants;
	}

	SceneComponent* SceneComponent::DeepDuplicate() const
	{
		SceneComponent* component = (SceneComponent*)Duplicate();
		for (SceneComponent*& child : component->m_Children)
		{
			child = child->DeepDuplicate();
			child->m_Parent = component;
		}
		return component;
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
			m_WorldTransformCache = m_RelativeTransform * m_Parent->GetWorldTransform();
		}
		else
		{
			if (GetOwner() && GetOwner()->GetParent())
			{
				ionassert(GetOwner()->GetParent()->GetRootComponent());
				m_WorldTransformCache = m_RelativeTransform * GetOwner()->GetParent()->GetWorldTransform();
			}
			else
			{
				m_WorldTransformCache = m_RelativeTransform;
			}
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

#pragma endregion

	MSceneComponent::MSceneComponent() :
		m_bVisible(true),
		m_bVisibleInGame(true)
	{
	}

	void MSceneComponent::OnCreate()
	{
	}

	void MSceneComponent::OnDestroy()
	{
	}

	void MSceneComponent::Tick(float deltaTime)
	{
	}

	void MSceneComponent::Attach(const TObjectPtr<MSceneComponent>& component)
	{
		ionassert(component);

		// @TODO: Make sure the component is not already attached somewhere in the hierarchy.

		m_ChildComponents.emplace_back(component);
	}
}
