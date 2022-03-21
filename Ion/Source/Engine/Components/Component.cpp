#include "IonPCH.h"

#include "Component.h"
#include "Engine/Entity.h"

namespace Ion
{
	// Component

	Component::Component() :
		m_bTickEnabled(true),
		m_WorldContext(nullptr),
		m_OwningEntity(nullptr),
		m_bIsSceneComponent(false)
		//m_bUpdateSceneData(false)
	{
	}

	void Component::SetTickEnabled(bool bTick)
	{
		m_bTickEnabled = bTick;
	}

	void Component::InitAsSceneComponent()
	{
		m_bIsSceneComponent = true;
		//m_bUpdateSceneData = true;
	}
	
	// ComponentRegistry

	ComponentRegistry::ComponentRegistry(World* worldContext) :
		m_WorldContext(worldContext)
	{
		ionassert(worldContext);
	}

	ComponentRegistry::~ComponentRegistry()
	{
		TRACE_FUNCTION();

		// Cleanup allocated structures
		for (auto& [id, ptr] : m_ComponentArrays)
		{
			checked_delete(ptr);
		}
	}

	void ComponentRegistry::Update(float deltaTime)
	{
		TRACE_FUNCTION();

		for (auto& [id, container] : m_ComponentArrays)
		{
			ComponentStaticCallbacks::TickFPtr func = ComponentStaticCallbacks::GetTickFPtr(id);
			checked_call(func, container, deltaTime);
		}
	}

	void ComponentRegistry::BuildRendererData(RRendererData& data)
	{
		TRACE_FUNCTION();

		for (auto& [id, container] : m_ComponentArrays)
		{
			ComponentStaticCallbacks::BuildRendererDataFPtr func = ComponentStaticCallbacks::GetBuildRendererDataFPtr(id);
			checked_call(func, container, data);
		}
	}

	THashSet<ComponentTypeID>* ComponentRegistry::s_RegisteredTypes = nullptr;
	ComponentTypeID ComponentRegistry::s_ComponentIDCounter = 0;
}
