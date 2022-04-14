#include "IonPCH.h"

#include "Entity.h"
#include "Engine/World.h"
#include "Engine/Components/SceneComponent.h"

namespace Ion
{
	Entity::Entity() :
		Entity(GUID())
	{
	}

	Entity::Entity(const GUID& guid) :
		m_GUID(guid),
		m_WorldContext(nullptr),
		m_Parent(nullptr),
		m_RootComponent(nullptr),
		m_bCreateEmptyRootOnSpawn(true),
		m_bTickEnabled(true),
		m_bPendingKill(false)
	{
		SetName("Entity");
	}

	void Entity::SetTransform(const Transform& transform)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetTransform(transform);

		UpdateChildrenWorldTransformCache();
	}

	void Entity::SetLocation(const Vector3& location)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetLocation(location);

		UpdateChildrenWorldTransformCache();
	}

	void Entity::SetRotation(const Rotator& rotation)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetRotation(rotation);

		UpdateChildrenWorldTransformCache();
	}

	void Entity::SetScale(const Vector3& scale)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetScale(scale);

		UpdateChildrenWorldTransformCache();
	}

	void Entity::SetRootComponent(SceneComponent* component)
	{
		// Reset the previous root first
		if (m_RootComponent)
		{
			UnbindComponent(m_RootComponent);
			m_RootComponent->UpdateWorldTransformCache();
			// @TODO: Temporary
			m_RootComponent->Destroy(false);
		}

		m_RootComponent = component;
		BindComponent(component);
		m_RootComponent->UpdateWorldTransformCache();
	}

	bool Entity::HasSceneComponent(SceneComponent* component) const
	{
		if (m_RootComponent == component)
			return true;

		return m_SceneComponents.find((SceneComponent*)component) != m_SceneComponents.end();

		//TArray<SceneComponent*> descendants = m_RootComponent->GetAllDescendants();
		//auto it = std::find(descendants.begin(), descendants.end(), component);
		//return it != descendants.end();
	}

	TArray<SceneComponent*> Entity::GetAllOwnedSceneComponents() const
	{
		return TArray<SceneComponent*>(m_SceneComponents.begin(), m_SceneComponents.end());
	}

	void Entity::AddComponent(Component* component)
	{
		ionassert(!component->IsSceneComponent(),
			"Add Scene Components using SceneComponent::AttachTo.");
		ionassert(!HasNonSceneComponent(component));

		BindComponent(component);
	}

	void Entity::RemoveComponent(Component* component)
	{
		ionassert(!component->IsSceneComponent(),
			"Remove Scene Components using SceneComponent::Detach.");
		ionassert(HasNonSceneComponent(component));

		UnbindComponent(component);
	}

	bool Entity::HasNonSceneComponent(Component* component) const
	{
		ionassert(!component->IsSceneComponent());
		return m_Components.find(component) != m_Components.end();
	}

	bool Entity::HasComponent(Component* component) const
	{
		if (!component)
			return false;

		if (component->IsSceneComponent())
		{
			return HasSceneComponent((SceneComponent*)component);
		}
		else
		{
			return HasNonSceneComponent(component);
		}

		//TArray<SceneComponent*> descendants = m_RootComponent->GetAllDescendants();
		//auto it = std::find(descendants.begin(), descendants.end(), component);
		//return it != descendants.end();
	}

	void Entity::AttachTo(Entity* parent)
	{
		if (!parent)
		{
			Detach();
			return;
		}

		ionassert(CanAttachTo(parent));

		if (m_Parent == parent)
		{
			LOG_WARN("The entity is already attached to {0} {{{1}}}.",
				m_Parent->GetName(),
				m_Parent->GetGuid().ToString());
			return;
		}

		// Update children in parents

		if (m_Parent)
			m_Parent->RemoveChild(this);

		parent->AddChild(this);

		m_Parent = parent;
		m_WorldContext->ReparentEntityInWorld(this, parent);

		UpdateWorldTransformCache();
	}

	void Entity::Detach()
	{
		if (m_Parent)
		{
			m_Parent->RemoveChild(this);
			m_Parent = nullptr;
			m_WorldContext->ReparentEntityInWorld(this, nullptr);

			UpdateWorldTransformCache();
		}
	}

	bool Entity::CanAttachTo(Entity* parent) const
	{
		// Can attach if parent isn't a part of the children
		TArray<Entity*> children = GetAllChildren();
		return std::find(children.begin(), children.end(), parent) == children.end();
	}

	void Entity::Update(float deltaTime)
	{
		if (m_bTickEnabled)
		{
			Tick(deltaTime);
		}
	}

	void Entity::SetNoCreateRootOnSpawn()
	{
		ionassert(!m_RootComponent, "Only call this in the constructor of a custom Entity class.");
		m_bCreateEmptyRootOnSpawn = false;
	}

	void Entity::Tick(float deltaTime)
	{

	}

	void Entity::OnSpawn(World* worldContext)
	{
		if (m_bCreateEmptyRootOnSpawn)
		{
			ComponentRegistry& registry = worldContext->GetComponentRegistry();
			SetRootComponent(registry.CreateComponent<EmptySceneComponent>());
			m_RootComponent->SetName("Root");
		}
	}

	void Entity::OnDestroy()
	{

	}

	void Entity::AddChild(Entity* child)
	{
		ionassert(child);
		ionassert(std::find(m_Children.begin(), m_Children.end(), child) == m_Children.end());

		m_Children.push_back(child);
	}

	void Entity::RemoveChild(Entity* child)
	{
		ionassert(child);

		auto it = std::find(m_Children.begin(), m_Children.end(), child);
		if (it != m_Children.end())
			m_Children.erase(it);
	}

	void Entity::GetAllChildren(TArray<Entity*>& outChildren) const
	{
		for (Entity* child : m_Children)
		{
			outChildren.push_back(child);
			child->GetAllChildren(outChildren);
		}
	}

	void Entity::BindComponent(Component* component)
	{
		ionassert(component);

		component->m_OwningEntity = this;
		if (component->IsSceneComponent())
		{
			SceneComponent* sceneComponent = (SceneComponent*)component;
			m_SceneComponents.insert(sceneComponent);
		}
		else
		{
			m_Components.insert(component);
		}
	}

	void Entity::UnbindComponent(Component* component)
	{
		ionassert(component);

		component->m_OwningEntity = nullptr;
		if (component->IsSceneComponent())
		{
			SceneComponent* sceneComponent = (SceneComponent*)component;
			m_SceneComponents.erase(sceneComponent);
		}
		else
		{
			m_Components.erase(component);
		}
	}

	void Entity::Destroy(bool bReparent)
	{
		ionassert(m_WorldContext);

		// Don't do anything if it's already set
		if (IsPendingKill())
			return;

		m_bPendingKill = true;

		// Reparent the children
		if (HasChildren())
		{
			// If it doesn't have a parent it will 
			// get parented to the world root anyway.
			Entity* attachTo = bReparent ?
				GetParent() : // World root if none
				nullptr;      // World root

			for (Entity* child : m_Children)
			{
				child->AttachTo(attachTo);
			}
		}

		Detach();

		// World is going to remove the entity from its collections
		m_WorldContext->MarkEntityForDestroy(this);

		// Destroy the components too

		// Can't use the for loop because Component::Destroy
		// removes the component from m_Components.
		while (!m_Components.empty())
		{
			Component* component = *m_Components.begin();
			component->Destroy();
		}
		m_Components.clear();

		// Children will get destroyed too
		m_RootComponent->Destroy(false);
		m_RootComponent = nullptr;
	}

	void Entity::DestroyWithChildren()
	{
		if (HasChildren())
		{
			for (Entity* child : m_Children)
			{
				child->DestroyWithChildren();
			}
		}

		Destroy(false);
	}

	void Entity::UpdateWorldTransformCache()
	{
		ionassert(m_RootComponent);
		m_RootComponent->UpdateWorldTransformCache();
		UpdateChildrenWorldTransformCache();
	}

	void Entity::UpdateChildrenWorldTransformCache()
	{
		for (Entity* child : m_Children)
		{
			child->UpdateWorldTransformCache();
		}
	}
}
