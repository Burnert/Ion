#include "IonPCH.h"

#include "Component.h"

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
		for (auto& [id, ptr] : m_Containers)
		{
			checked_delete(ptr);
		}
	}

	void ComponentRegistry::Update(float deltaTime)
	{
		TRACE_FUNCTION();

		// Call the functions for each component container
		for (auto& [id, container] : m_Containers)
		{
			ComponentSerialCall::Tick(id, container, deltaTime);
		}
	}

	void ComponentRegistry::BuildRendererData(RRendererData& data)
	{
		TRACE_FUNCTION();

		for (auto& [id, container] : m_Containers)
		{
			ComponentSerialCall::BuildRendererData(id, container, data);
		}
	}
}
