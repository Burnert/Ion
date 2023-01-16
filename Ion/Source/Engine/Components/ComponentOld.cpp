#include "IonPCH.h"

#include "ComponentOld.h"
#include "SceneComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/World.h"

#include "BehaviorComponent.h"
#include "DirectionalLightComponent.h"
#include "LightComponent.h"
#include "MeshComponent.h"

namespace Ion
{
	// Component

	ComponentOld::ComponentOld() :
		m_bTickEnabled(true),
		m_WorldContext(nullptr),
		m_OwningEntity(nullptr),
		m_bIsSceneComponent(false),
		m_bPendingKill(false)
		//m_bUpdateSceneData(false)
	{
	}

	void ComponentOld::OnCreate()
	{
	}

	void ComponentOld::OnDestroy()
	{
		
	}

	ComponentOld* ComponentOld::Duplicate() const
	{
		ionassert(m_WorldContext);
		ComponentOld* copy = Duplicate_Internal(m_WorldContext->GetComponentRegistry());
		return copy;
	}

	void ComponentOld::Destroy(bool bReparent)
	{
		ionassert(m_WorldContext);

		// Don't do anything if it's already set
		if (IsPendingKill())
			return;

		m_bPendingKill = true;

		if (m_OwningEntity)
		{
			if (IsSceneComponent())
			{
				SceneComponent* comp = ((SceneComponent*)this);
				if (comp->HasChildren())
				{
					if (bReparent && !comp->GetParent())
					{
						ComponentLogger.Error("Cannot reparent component's children if it has no parent.");
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

		m_WorldContext->GetComponentRegistry().MarkForDestroy(this);
	}

	void ComponentOld::SetTickEnabled(bool bTick)
	{
		m_bTickEnabled = bTick;
	}

	void ComponentOld::InitAsSceneComponent()
	{
		m_bIsSceneComponent = true;
		//m_bUpdateSceneData = true;
	}
	
	// ComponentRegistry

	ComponentRegistry::ComponentRegistry(World* worldContext) :
		m_WorldContext(worldContext)
	{
		ionassert(worldContext);
		RegisterComponents();
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

	void ComponentRegistry::RegisterComponents()
	{
		TRACE_FUNCTION();

		ComponentLogger.Debug("ComponentRegistry::RegisterComponents");

		RegisterComponentClass<BehaviorComponent>();
		RegisterComponentClass<EmptySceneComponent>();
		RegisterComponentClass<DirectionalLightComponent>();
		RegisterComponentClass<LightComponent>();
		RegisterComponentClass<MeshComponent>();
	}

	ComponentOld* ComponentRegistry::FindComponentByGUID(const GUID& guid) const
	{
		auto it = m_ComponentsByGUID.find(guid);
		if (it != m_ComponentsByGUID.end())
			return it->second;
		return nullptr;
	}

	void ComponentRegistry::MarkForDestroy(ComponentOld* component)
	{
		ionassert(component->IsPendingKill());
		ionassert(m_InvalidComponents.find(component->GetFinalTypeID()) != m_InvalidComponents.end());

		m_InvalidComponents.at(component->GetFinalTypeID()).push_back(component);
	}

	void ComponentRegistry::DestroyInvalidComponents()
	{
		// Invalid components are those that are pending kill
		// => that Destroy has been called on.
		// They are added to m_InvalidComponents
		// using the MarkForDestroy function.
		for (auto& [id, container] : m_InvalidComponents)
		{
			for (ComponentOld* invalidComponent : container)
			{
				ionassert(id == invalidComponent->GetFinalTypeID());
				DestroyComponent(invalidComponent);
			}
			container.clear();
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
