#include "IonPCH.h"

#include "Entity.h"
#include "World.h"

namespace Ion
{
	Entity::Entity() :
		m_WorldContext(nullptr),
		m_bTickEnabled(true),
		m_bVisible(true),
		m_bVisibleInGame(true),
		m_Parent(nullptr)
	{
	}

	Entity::Entity(const GUID& guid) :
		m_GUID(guid),
		m_WorldContext(nullptr),
		m_bTickEnabled(true),
		m_bVisible(true),
		m_bVisibleInGame(true),
		m_Parent(nullptr)
	{
	}

	void Entity::AddComponent(Component* component)
	{
		ionassert(m_WorldContext);
		ionassert(m_Components.find(component) == m_Components.end());

		m_WorldContext->GetComponentRegistry().BindComponentToEntity(this, component);
		component->m_OwningEntity = this;
		m_Components.insert(component);
	}

	void Entity::RemoveComponent(Component* component)
	{
		ionassert(m_WorldContext);
		ionassert(m_Components.find(component) != m_Components.end());

		m_WorldContext->GetComponentRegistry().UnbindComponentFromEntity(this, component);
		component->m_OwningEntity = nullptr;
		m_Components.erase(component);
	}

	void Entity::Update(float deltaTime)
	{
		if (m_bTickEnabled)
		{
			Tick(deltaTime);
		}
	}

	void Entity::AddChild(Entity* child)
	{
		m_Children.push_back(child);
	}

	void Entity::RemoveChild(Entity* child)
	{
		auto it = std::find(m_Children.begin(), m_Children.end(), child);
		if (it != m_Children.end())
			m_Children.erase(it);
	}

	void Entity::AttachTo(Entity* parent)
	{
		// Update children in parents

		if (m_Parent)
			m_Parent->RemoveChild(this);

		if (parent)
			parent->AddChild(this);

		// Parent can be null
		m_Parent = parent;
		
		m_WorldContext->ReparentEntityInWorld(this, parent);
	}
}
