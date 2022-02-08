#include "IonPCH.h"

#include "Entity.h"
#include "World.h"

namespace Ion
{
	Entity::Entity() :
		m_WorldContext(nullptr),
		m_bTickEnabled(true)
	{
	}

	Entity::Entity(const GUID& guid) :
		m_GUID(guid),
		m_WorldContext(nullptr),
		m_bTickEnabled(true)
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
}
