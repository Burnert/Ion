#pragma once

#include "Engine/SceneObjectData.h"

namespace Ion
{
	class ION_API Entity
	{
	public:
		using ComponentSet = THashSet<Component*>;

		Entity();
		Entity(const GUID& guid);

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
		void AddComponent(Component* component);
		/* Removes a non-scene component */
		void RemoveComponent(Component* component);
		bool HasNonSceneComponent(Component* component) const;
		const ComponentSet& GetComponents() const;

		bool HasComponent(Component* component) const;

		/* Set entity related component data and add the component to entity's collection. */
		void BindComponent(Component* component);
		/* Reset entity related component data and remove the component from entity's collection. */
		void UnbindComponent(Component* component);

		void SetName(const String& name);
		const String& GetName() const;

		/* If bReparent is true, the child entities will get reparented
		   to this entity's parent instead of the world root, unless it has no parent.
		   Use DestroyWithChildren if you don't want to keep the children. */
		void Destroy(bool bReparent = true);
		void DestroyWithChildren();

		/* Returns null if the Entity is parented to the World root. */
		Entity* GetParent() const;
		/* If the parent paremeter is null, the Entity will be parented to the World root. */
		void AttachTo(Entity* parent);
		/* Parent to the World root. */
		void Detach();

		bool HasChildren() const;
		const TArray<Entity*>& GetChildren() const;

		/* Returns the GUID of the Entity.
		   A GUID is initiated at the creation of the Entity. */
		const GUID& GetGuid() const;

		/* Returns a pointer to the World the Entity is currently in. */
		World* GetWorldContext() const;

		Entity(const Entity&) = delete;
		Entity(Entity&&) noexcept = delete;
		Entity& operator=(const Entity&) = delete;
		Entity& operator=(Entity&&) = delete;

		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;

	protected:
		void Update(float deltaTime);

		/* Call in custom entity class constructor. */
		void SetNoCreateRootOnSpawn();

	protected:
		/* Called each frame. */
		virtual void Tick(float deltaTime);
		/* Called when the entity is spawned in the world. */
		virtual void OnSpawn(World* worldContext);
		/* Called when the entity is removed from the world. */
		virtual void OnDestroy();

	private:
		void AddChild(Entity* child);
		void RemoveChild(Entity* child);

		/* Called in any function that changes the relative transform. */
		void UpdateWorldTransformCache();
		void UpdateChildrenWorldTransformCache();
		void UpdateRootComponentWorldTransformCache();

	private:
		GUID m_GUID;

		World* m_WorldContext;

		SceneObjectData m_SceneData;

		// Only non-scene components
		ComponentSet m_Components;
		SceneComponent* m_RootComponent;
		THashSet<SceneComponent*> m_SceneComponents;

		Entity* m_Parent;
		TArray<Entity*> m_Children;

		String m_Name;

		uint8 m_bCreateEmptyRootOnSpawn : 1;
		uint8 m_bTickEnabled : 1;

		friend class Engine;
		friend class World;
	};

	// Inline definitions

	inline const Transform& Entity::GetTransform() const
	{
		return m_SceneData.RelativeTransform;
	}

	inline const Vector3& Entity::GetLocation() const
	{
		return m_SceneData.RelativeTransform.GetLocation();
	}

	inline const Rotator& Entity::GetRotation() const
	{
		return m_SceneData.RelativeTransform.GetRotation();
	}

	inline const Vector3& Entity::GetScale() const
	{
		return m_SceneData.RelativeTransform.GetScale();
	}

	inline const Transform& Entity::GetWorldTransform() const
	{
		return m_SceneData.WorldTransformCache;
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

	inline void Entity::SetName(const String& name)
	{
		m_Name = name;
	}

	inline const String& Entity::GetName() const
	{
		return m_Name;
	}

	inline Entity* Entity::GetParent() const
	{
		return m_Parent;
	}

	inline bool Entity::HasChildren() const
	{
		return !m_Children.empty();
	}

	inline const TArray<Entity*>& Entity::GetChildren() const
	{
		return m_Children;
	}

	inline const GUID& Entity::GetGuid() const
	{
		return m_GUID;
	}

	inline World* Entity::GetWorldContext() const
	{
		return m_WorldContext;
	}

	inline bool Entity::operator==(const Entity& other) const
	{
		return m_GUID == other.m_GUID;
	}

	inline bool Entity::operator!=(const Entity& other) const
	{
		return m_GUID != other.m_GUID;
	}
}
