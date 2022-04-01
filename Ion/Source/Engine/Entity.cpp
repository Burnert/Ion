#include "IonPCH.h"

#include "Entity.h"
#include "World.h"
#include "Components/SceneComponent.h"

namespace Ion
{
	Entity::Entity() : Entity(GUID())
	{
	}

	Entity::Entity(const GUID& guid) :
		m_GUID(guid),
		m_WorldContext(nullptr),
		m_bCreateEmptyRootOnSpawn(true),
		m_bTickEnabled(true),
		m_Parent(nullptr),
		m_RootComponent(nullptr)
		//m_RootComponentNode(m_ComponentTreeNodeFactory.Create(nullptr))
	{
	}

	void Entity::SetTransform(const Transform& transform)
	{
		m_SceneData.RelativeTransform = transform;
		UpdateWorldTransformCache();
	}

	void Entity::SetLocation(const Vector3& location)
	{
		m_SceneData.RelativeTransform.SetLocation(location);
		UpdateWorldTransformCache();
	}

	void Entity::SetRotation(const Rotator& rotation)
	{
		m_SceneData.RelativeTransform.SetRotation(rotation);
		UpdateWorldTransformCache();
	}

	void Entity::SetScale(const Vector3& scale)
	{
		m_SceneData.RelativeTransform.SetScale(scale);
		UpdateWorldTransformCache();
	}

	void Entity::SetRootComponent(SceneComponent* component)
	{
		// Reset the previous root first
		// @TODO: POTENTIAL MEMORY LEAK
		// when the previous root component gets abandoned
		if (m_RootComponent)
			UnbindComponent(m_RootComponent);

		m_RootComponent = component;
		BindComponent(m_RootComponent);
	}

	bool Entity::HasSceneComponent(SceneComponent* component) const
	{
		if (m_RootComponent == component)
			return true;

		TArray<SceneComponent*> descendants = m_RootComponent->GetAllDescendants();
		auto it = std::find(descendants.begin(), descendants.end(), component);
		return it != descendants.end();
	}

	void Entity::AddComponent(Component* component)
	{
		ionassert(!component->IsSceneComponent(),
			"Add Scene Components using SceneComponent::AttachTo.");
		ionassert(!HasNonSceneComponent(component));

		//m_WorldContext->GetComponentRegistry().BindComponentToEntity(this, component);
		BindComponent(component);
		m_Components.insert(component);
	}

	void Entity::RemoveComponent(Component* component)
	{
		ionassert(!component->IsSceneComponent(),
			"Remove Scene Components using SceneComponent::Detach.");
		ionassert(HasNonSceneComponent(component));

		//m_WorldContext->GetComponentRegistry().UnbindComponentFromEntity(this, component);
		UnbindComponent(component);
		m_Components.erase(component);
	}

	bool Entity::HasNonSceneComponent(Component* component) const
	{
		ionassert(!component->IsSceneComponent());
		return m_Components.find(component) != m_Components.end();
	}

	bool Entity::HasComponent(Component* component) const
	{
		if (!component || !m_RootComponent)
			return false;

		TArray<SceneComponent*> descendants = m_RootComponent->GetAllDescendants();
		auto it = std::find(descendants.begin(), descendants.end(), component);
		return it != descendants.end();
	}

	void Entity::AttachTo(Entity* parent)
	{
		if (!parent)
		{
			Detach();
		}

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
	}

	void Entity::Detach()
	{
		if (m_Parent)
		{
			m_Parent->RemoveChild(this);
			m_Parent = nullptr;
			m_WorldContext->ReparentEntityInWorld(this, nullptr);
		}
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

	void Entity::OnSpawn(World* worldContext)
	{
		if (m_bCreateEmptyRootOnSpawn)
		{
			ComponentRegistry& registry = worldContext->GetComponentRegistry();
			SetRootComponent(registry.CreateComponent<EmptySceneComponent>());
			m_RootComponent->SetName("Root");
		}
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

	void Entity::BindComponent(Component* component)
	{
		ionassert(component);

		component->m_OwningEntity = this;
	}

	void Entity::UnbindComponent(Component* component)
	{
		ionassert(component);

		component->m_OwningEntity = nullptr;
	}

	void Entity::UpdateWorldTransformCache()
	{
		if (m_Parent)
		{
			m_SceneData.WorldTransformCache = m_SceneData.RelativeTransform * m_Parent->GetWorldTransform();
		}
		else
		{
			m_SceneData.WorldTransformCache = m_SceneData.RelativeTransform;
		}

		UpdateChildrenWorldTransformCache();
		UpdateRootComponentWorldTransformCache();
	}

	void Entity::UpdateChildrenWorldTransformCache()
	{
		for (Entity* child : m_Children)
		{
			child->UpdateWorldTransformCache();
		}
	}

	void Entity::UpdateRootComponentWorldTransformCache()
	{
		if (m_RootComponent)
		{
			m_RootComponent->UpdateWorldTransformCache();
		}
	}
}
