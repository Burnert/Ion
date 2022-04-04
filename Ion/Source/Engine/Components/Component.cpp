#include "IonPCH.h"

#include "Component.h"
#include "SceneComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/World.h"

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

	void Component::Destroy(bool bReparent)
	{
		ionassert(m_WorldContext);

		if (m_OwningEntity)
		{
			if (IsSceneComponent())
			{
				SceneComponent* comp = ((SceneComponent*)this);
				if (comp->HasChildren())
				{
					if (bReparent && !comp->GetParent())
					{
						LOG_ERROR("Cannot reparent component's children if it has no parent.");
						bReparent = false; // Force no reparent
					}

					// Reparent or destroy the children
					if (bReparent)
					{
						for (SceneComponent* child : comp->GetChildren())
						{
							child->AttachTo(comp->GetParent());
						}
					}
					else
					{
						for (SceneComponent* child : comp->GetChildren())
						{
							child->Destroy(false);
						}
					}
				}
				comp->Detach();
			}
			else
			{
				m_OwningEntity->RemoveComponent(this);
			}
		}

		m_WorldContext->GetComponentRegistry().DestroyComponent(this);
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
