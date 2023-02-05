#include "IonPCH.h"

#include "EntityOld.h"
#include "MeshEntity.h"
#include "Engine/World.h"
#include "Engine/Components/SceneComponent.h"

namespace Ion
{
	EntityOld::EntityOld() :
		m_WorldContext(nullptr),
		m_Parent(nullptr),
		m_RootComponent(nullptr),
		m_bCreateEmptyRootOnSpawn(true),
		m_bTickEnabled(true),
		m_bPendingKill(false)
	{
		m_ClassName = "Entity";
	}

	void EntityOld::SetTransform(const Transform& transform)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetTransform(transform);

		UpdateChildrenWorldTransformCache();
	}

	void EntityOld::SetLocation(const Vector3& location)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetLocation(location);

		UpdateChildrenWorldTransformCache();
	}

	void EntityOld::SetRotation(const Rotator& rotation)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetRotation(rotation);

		UpdateChildrenWorldTransformCache();
	}

	void EntityOld::SetScale(const Vector3& scale)
	{
		ionassert(m_RootComponent);
		m_RootComponent->SetScale(scale);

		UpdateChildrenWorldTransformCache();
	}

	void EntityOld::SetRootComponent(SceneComponent* component)
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

	bool EntityOld::HasSceneComponent(SceneComponent* component) const
	{
		if (m_RootComponent == component)
			return true;

		return m_SceneComponents.find((SceneComponent*)component) != m_SceneComponents.end();

		//TArray<SceneComponent*> descendants = m_RootComponent->GetAllDescendants();
		//auto it = std::find(descendants.begin(), descendants.end(), component);
		//return it != descendants.end();
	}

	TArray<SceneComponent*> EntityOld::GetAllOwnedSceneComponents() const
	{
		return TArray<SceneComponent*>(m_SceneComponents.begin(), m_SceneComponents.end());
	}

	void EntityOld::AddComponent(ComponentOld* component)
	{
		ionassert(!component->IsSceneComponent(),
			"Add Scene Components using SceneComponent::AttachTo.");
		ionassert(!HasNonSceneComponent(component));

		BindComponent(component);
	}

	void EntityOld::RemoveComponent(ComponentOld* component)
	{
		ionassert(!component->IsSceneComponent(),
			"Remove Scene Components using SceneComponent::Detach.");
		ionassert(HasNonSceneComponent(component));

		UnbindComponent(component);
	}

	bool EntityOld::HasNonSceneComponent(ComponentOld* component) const
	{
		ionassert(!component->IsSceneComponent());
		return m_Components.find(component) != m_Components.end();
	}

	bool EntityOld::HasComponent(ComponentOld* component) const
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

	void EntityOld::AttachTo(const TObjectPtr<EntityOld>& parent)
	{
		if (!parent)
		{
			Detach();
			return;
		}

		ionassert(CanAttachTo(parent));

		if (m_Parent == parent)
		{
			EntityLogger.Warn("The entity is already attached to {0} {{{1}}}.",
				m_Parent->GetName(),
				m_Parent->GetGuid().ToString());
			return;
		}

		// Update children in parents
		
		if (m_Parent)
			m_Parent->RemoveChild(AsPtr());

		parent->AddChild(AsPtr());

		m_Parent = parent;
		m_WorldContext->ReparentEntityInWorld(AsPtr(), parent);

		UpdateWorldTransformCache();
	}

	void EntityOld::Detach()
	{
		if (m_Parent)
		{
			m_Parent->RemoveChild(AsPtr());
			m_Parent = nullptr;
			m_WorldContext->ReparentEntityInWorld(AsPtr(), nullptr);

			UpdateWorldTransformCache();
		}
	}

	bool EntityOld::CanAttachTo(const TObjectPtr<EntityOld>& parent) const
	{
		// Can attach if parent isn't a part of the children
		TArray<TObjectPtr<EntityOld>> children = GetAllChildren();
		return std::find(children.begin(), children.end(), parent) == children.end();
	}

	void EntityOld::Update(float deltaTime)
	{
		if (m_bTickEnabled)
		{
			Tick(deltaTime);
		}
	}

	void EntityOld::SetNoCreateRootOnSpawn()
	{
		ionassert(!m_RootComponent, "Only call this in the constructor of a custom EntityOld class.");
		m_bCreateEmptyRootOnSpawn = false;
	}

	void EntityOld::Tick(float deltaTime)
	{

	}

	void EntityOld::OnSpawn(World* worldContext)
	{
		if (m_bCreateEmptyRootOnSpawn)
		{
			ComponentRegistry& registry = worldContext->GetComponentRegistry();
			SetRootComponent(registry.CreateComponent<EmptySceneComponent>());
			m_RootComponent->SetName("Root");
		}
	}

	void EntityOld::OnDestroy()
	{

	}

	void EntityOld::Serialize(Archive& ar)
	{
		Super::Serialize(ar);

		XMLArchiveAdapter xmlAr = ar;

		// Save parents and children
		// Relationship needs to be setup after all the entities are available,
		// which might not be the case right now.
		if (ar.IsSaving())
		{
			xmlAr.EnterNode("Parent" IASSET_NODE_Guid);
			GUID parentGuid = m_Parent ? m_Parent->GetGuid() : GUID::Zero;
			ar &= parentGuid;
			xmlAr.ExitNode();

			TArray<GUID> childrenGuids;
			childrenGuids.reserve(m_Children.size());
			std::transform(m_Children.begin(), m_Children.end(), std::back_inserter(childrenGuids), [](const TObjectPtr<EntityOld>& ent)
			{
				return ent->GetGuid();
			});
		}

		// @TODO: Temporary solution, make some kind of a field serializer or something
		xmlAr.EnterNode("SceneData");
		ar &= m_SceneData;
		xmlAr.ExitNode();

		xmlAr.EnterNode("CreateEmptyRootOnSpawn");
		SERIALIZE_BIT_FIELD(ar, m_bCreateEmptyRootOnSpawn);
		xmlAr.ExitNode();
		xmlAr.EnterNode("TickEnabled");
		SERIALIZE_BIT_FIELD(ar, m_bTickEnabled);
		xmlAr.ExitNode();

		// @TODO: serialize components
	}

	void EntityOld::AddChild(const TObjectPtr<EntityOld>& child)
	{
		ionassert(child);
		ionassert(std::find(m_Children.begin(), m_Children.end(), child) == m_Children.end());

		m_Children.push_back(child);
	}

	void EntityOld::RemoveChild(const TObjectPtr<EntityOld>& child)
	{
		ionassert(child);

		auto it = std::find(m_Children.begin(), m_Children.end(), child);
		if (it != m_Children.end())
			m_Children.erase(it);
	}

	void EntityOld::GetAllChildren(TArray<TObjectPtr<EntityOld>>& outChildren) const
	{
		for (const TObjectPtr<EntityOld>& child : m_Children)
		{
			outChildren.push_back(child);
			child->GetAllChildren(outChildren);
		}
	}

	void EntityOld::BindComponent(ComponentOld* component)
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

	void EntityOld::UnbindComponent(ComponentOld* component)
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

	EntityOld* EntityOld::Duplicate() const
	{
		ionassert(!IsPendingKill());
		EntityOld* newEntity = Duplicate_Internal();

		//newEntity->m_GUID = GUID();

		// Children would need to be duplicated too.
		// Delete the references from the new entity instead.
		newEntity->m_Children.clear();

		// Duplicate the non-scene components

		THashSet<ComponentOld*> newComponents;
		for (const ComponentOld* const& component : newEntity->m_Components)
		{
			auto [it, bUnique] = newComponents.insert(component->Duplicate());
			(*it)->m_OwningEntity = newEntity;
		}
		newEntity->m_Components.swap(newComponents);

		// Duplicate the scene components

		newEntity->m_SceneComponents.clear();

		newEntity->m_RootComponent = newEntity->m_RootComponent->DeepDuplicate();
		newEntity->BindComponent(newEntity->m_RootComponent);
		
		for (SceneComponent* component : newEntity->m_RootComponent->GetAllDescendants())
		{
			newEntity->BindComponent(component);
		}

		return newEntity;
	}

	EntityOld* EntityOld::Duplicate_Internal() const
	{
		return nullptr;// new EntityOld(*this);
	}

	void EntityOld::Destroy(bool bReparent)
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
			TObjectPtr<EntityOld> attachTo = bReparent ?
				GetParent() : // World root if none
				nullptr;      // World root

			for (const TObjectPtr<EntityOld>& child : m_Children)
			{
				child->AttachTo(attachTo);
			}
		}

		Detach();

		// World is going to remove the entity from its collections
		m_WorldContext->MarkEntityForDestroy(AsPtr());

		// Destroy the components too

		// Can't use the for loop because ComponentOld::Destroy
		// removes the component from m_Components.
		while (!m_Components.empty())
		{
			ComponentOld* component = *m_Components.begin();
			component->Destroy();
		}
		m_Components.clear();

		// Children will get destroyed too
		m_RootComponent->Destroy(false);
		m_RootComponent = nullptr;
	}

	void EntityOld::DestroyWithChildren()
	{
		if (HasChildren())
		{
			for (const TObjectPtr<EntityOld>& child : m_Children)
			{
				child->DestroyWithChildren();
			}
		}

		Destroy(false);
	}

	void EntityOld::UpdateWorldTransformCache()
	{
		ionassert(m_RootComponent);
		m_RootComponent->UpdateWorldTransformCache();
		UpdateChildrenWorldTransformCache();
	}

	void EntityOld::UpdateChildrenWorldTransformCache()
	{
		for (const TObjectPtr<EntityOld>& child : m_Children)
		{
			child->UpdateWorldTransformCache();
		}
	}
}
