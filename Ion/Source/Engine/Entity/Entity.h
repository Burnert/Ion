#pragma once

#include "Engine/EngineCore.h"
#include "Engine/SceneObjectData.h"
#include "Engine/Components/SceneComponent.h"
#include "Matter/Object.h"

namespace Ion
{
	REGISTER_LOGGER(EntityLogger, "Engine::ECS::Entity");

	class ION_API Entity : public MObject
	{
		MCLASS(Entity)
		using Super = MObject;

		using ComponentSet = THashSet<ComponentOld*>;

		Entity();

		void SetTransform(const Transform& transform);
		const Transform& GetTransform() const;

		void SetLocation(const Vector3& location);
		const Vector3& GetLocation() const;

		void SetRotation(const Rotator& rotation);
		const Rotator& GetRotation() const;

		void SetScale(const Vector3& scale);
		const Vector3& GetScale() const;

		const Transform& GetWorldTransform() const;

		void SetVisible(bool bVisible);
		bool IsVisible() const;

		void SetVisibleInGame(bool bVisibleInGame);
		bool IsVisibleInGame() const;

		void SetTickEnabled(bool bTick);
		bool IsTickEnabled() const;

		void SetRootComponent(SceneComponent* component);
		SceneComponent* GetRootComponent() const;
		bool HasSceneComponent(SceneComponent* component) const;
		TArray<SceneComponent*> GetAllOwnedSceneComponents() const;

		/* Adds a non-scene component */
		void AddComponent(ComponentOld* component);
		/* Removes a non-scene component */
		void RemoveComponent(ComponentOld* component);
		bool HasNonSceneComponent(ComponentOld* component) const;
		const ComponentSet& GetComponents() const;

		bool HasComponent(ComponentOld* component) const;

		/* Set entity related component data and add the component to entity's collection. 
		   Updates scene component's world transform cache. */
		void BindComponent(ComponentOld* component);
		/* Reset entity related component data and remove the component from entity's collection.
		   Updates scene component's world transform cache. */
		void UnbindComponent(ComponentOld* component);

		Entity* Duplicate() const;
	protected:
		virtual Entity* Duplicate_Internal() const;
	public:
		/* If bReparent is true, the child entities will get reparented
		   to this entity's parent instead of the world root, unless it has no parent.
		   Use DestroyWithChildren if you don't want to keep the children. */
		void Destroy(bool bReparent = true);
		void DestroyWithChildren();

		bool IsPendingKill() const;

		bool HasParent() const;
		/* Returns null if the Entity is parented to the World root. */
		const TObjectPtr<Entity>& GetParent() const;
		/* If the parent paremeter is null, the Entity will be parented to the World root. */
		void AttachTo(const TObjectPtr<Entity>& parent);
		/* Parent to the World root. */
		void Detach();
		bool CanAttachTo(const TObjectPtr<Entity>& parent) const;

		bool HasChildren() const;
		const TArray<TObjectPtr<Entity>>& GetChildren() const;
		TArray<TObjectPtr<Entity>> GetAllChildren() const;

		/* Returns a pointer to the World the Entity is currently in. */
		World* GetWorldContext() const;

	protected:
		void Update(float deltaTime);

		/* Call in custom entity class constructor. */
		void SetNoCreateRootOnSpawn();

	protected:
		/* Called each frame. */
		virtual void Tick(float deltaTime);
		/* Called when the entity is spawned in the world. */
		virtual void OnSpawn(World* worldContext);
		/* Called when the entity is removed from the world. 
		   Destroys all the owned components. */
		virtual void OnDestroy();

	public:
		virtual void Serialize(Archive& ar) override;

	private:
		void AddChild(const TObjectPtr<Entity>& child);
		void RemoveChild(const TObjectPtr<Entity>& child);

		void GetAllChildren(TArray<TObjectPtr<Entity>>& outChildren) const;

		/* Called in any function that changes the relative transform. */
		void UpdateWorldTransformCache();
		void UpdateChildrenWorldTransformCache();

	protected:
		// @TODO: Very temporary, without reflection it's pretty much impossible to do properly.
		String m_ClassName;

	private:
		World* m_WorldContext;

		SceneObjectData m_SceneData;

		// Only non-scene components
		ComponentSet m_Components;
		SceneComponent* m_RootComponent;
		THashSet<SceneComponent*> m_SceneComponents;

		TObjectPtr<Entity> m_Parent;
		TArray<TObjectPtr<Entity>> m_Children;

		uint8 m_bCreateEmptyRootOnSpawn : 1;
		uint8 m_bTickEnabled : 1;
		uint8 m_bPendingKill : 1;

		friend class Engine;
		friend class World;
	};

	// Inline definitions

	inline const Transform& Entity::GetTransform() const
	{
		ionassert(m_RootComponent);
		return m_RootComponent->GetTransform();
	}

	inline const Vector3& Entity::GetLocation() const
	{
		ionassert(m_RootComponent);
		return m_RootComponent->GetLocation();
	}

	inline const Rotator& Entity::GetRotation() const
	{
		ionassert(m_RootComponent);
		return m_RootComponent->GetRotation();
	}

	inline const Vector3& Entity::GetScale() const
	{
		ionassert(m_RootComponent);
		return m_RootComponent->GetScale();
	}

	inline const Transform& Entity::GetWorldTransform() const
	{
		ionassert(m_RootComponent);
		return m_RootComponent->GetWorldTransform();
	}

	inline void Entity::SetVisible(bool bVisible)
	{
		m_SceneData.bVisible = bVisible;
	}

	inline bool Entity::IsVisible() const
	{
		return m_SceneData.bVisible;
	}

	inline void Entity::SetVisibleInGame(bool bVisibleInGame)
	{
		m_SceneData.bVisibleInGame = bVisibleInGame;
	}

	inline bool Entity::IsVisibleInGame() const
	{
		return m_SceneData.bVisibleInGame;
	}

	inline void Entity::SetTickEnabled(bool bTick)
	{
		m_bTickEnabled = bTick;
	}

	inline bool Entity::IsTickEnabled() const
	{
		return m_bTickEnabled;
	}

	inline SceneComponent* Entity::GetRootComponent() const
	{
		return m_RootComponent;
	}

	inline const Entity::ComponentSet& Entity::GetComponents() const
	{
		return m_Components;
	}

	inline bool Entity::HasParent() const
	{
		return m_Parent;
	}

	inline const TObjectPtr<Entity>& Entity::GetParent() const
	{
		return m_Parent;
	}

	inline bool Entity::HasChildren() const
	{
		return !m_Children.empty();
	}

	inline const TArray<TObjectPtr<Entity>>& Entity::GetChildren() const
	{
		return m_Children;
	}

	inline TArray<TObjectPtr<Entity>> Entity::GetAllChildren() const
	{
		TArray<TObjectPtr<Entity>> children;
		GetAllChildren(children);
		return children;
	}

	inline World* Entity::GetWorldContext() const
	{
		return m_WorldContext;
	}

	inline bool Entity::IsPendingKill() const
	{
		return m_bPendingKill;
	}
}
