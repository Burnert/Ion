#include "IonPCH.h"

#include "Component.h"

namespace Ion
{
	// Component

	Component::Component() :
		m_bTickEnabled(true),
		m_WorldContext(nullptr),
		m_OwningEntity(nullptr),
		m_bIsSceneComponent(false),
		m_bUpdateSceneData(false)
	{
	}

	void Component::SetTickEnabled(bool bTick)
	{
		m_bTickEnabled = bTick;
	}

	bool Component::IsTickEnabled() const
	{
		return m_bTickEnabled;
	}

	void Component::InitAsSceneComponent()
	{
		m_bIsSceneComponent = true;
		m_bUpdateSceneData = true;
	}
	
	// ComponentRegistry

	ComponentRegistry::ComponentRegistry(World* worldContext) :
		m_WorldContext(worldContext)
	{
		ionassert(worldContext);
	}

	ComponentRegistry::~ComponentRegistry()
	{
		// Cleanup allocated structures
		for (auto& [id, ptr] : m_ComponentArrays)
		{
			checked_delete(ptr);
		}
	}

	void ComponentRegistry::Update(float deltaTime)
	{
		for (auto& [id, container] : m_ComponentArrays)
		{
			ComponentCallbacks::TickFPtr func = ComponentCallbacks::GetTickFPtr(id);
			checked_call(func, container, deltaTime);
		}
	}

	THashSet<ComponentTypeID> ComponentRegistry::s_RegisteredTypes;
	ComponentTypeID ComponentRegistry::s_ComponentIDCounter = 0;
}
