#pragma once

namespace Ion
{
	class ION_API Entity
	{
	public:
		//using ComponentTreeNodeFactory = TTreeNodeFactory<Component*, 32>;
		//using ComponentTreeNode        = TTreeNode<Component*>;
		//using ComponentNodeMap         = THashMap<Component*, ComponentTreeNode*>;

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

		Transform GetWorldTransform() const;

		void SetVisible(bool bVisible);
		bool IsVisible() const;

		void SetVisibleInGame(bool bVisibleInGame);
		bool IsVisibleInGame() const;

		void SetTickEnabled(bool bTick);
		bool IsTickEnabled() const;

		void SetRootComponent(SceneComponent* component);
		SceneComponent* GetRootComponent() const;
		bool HasSceneComponent(SceneComponent* component) const;

		/* Adds a non-scene component */
		void AddComponent(Component* component);
		/* Removes a non-scene component */
		void RemoveComponent(Component* component);
		bool HasNonSceneComponent(Component* component) const;
		const ComponentSet& GetComponents() const;

		bool HasComponent(Component* component) const;

		/* Set entity related component data. */
		void BindComponent(Component* component);
		/* Reset entity related component data. */
		void UnbindComponent(Component* component);

		void SetName(const String& name);
		const String& GetName() const;

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
		virtual void Tick(float deltaTime) { }
		virtual void OnSpawn(World* worldContext);
		virtual void OnDestroy() { }

	private:
		void AddChild(Entity* child);
		void RemoveChild(Entity* child);

		/* Called in any function that changes the relative transform. */
		void UpdateWorldTransformCache();
		void UpdateChildrenWorldTransformCache();

	private:
		GUID m_GUID;

		World* m_WorldContext;

		Transform m_RelativeTransform;
		Transform m_WorldTransformCache;

		SceneComponent* m_RootComponent;
		ComponentSet m_Components;

		Entity* m_Parent;
		TArray<Entity*> m_Children;

		String m_Name;

		uint8 m_bCreateEmptyRootOnSpawn : 1;
		uint8 m_bTickEnabled : 1;
		uint8 m_bVisible : 1;
		uint8 m_bVisibleInGame : 1;

		friend class Engine;
		friend class World;
	};

	// Inline definitions

	inline const Transform& Entity::GetTransform() const
	{
		return m_RelativeTransform;
	}

	inline const Vector3& Entity::GetLocation() const
	{
		return m_RelativeTransform.GetLocation();
	}

	inline const Rotator& Entity::GetRotation() const
	{
		return m_RelativeTransform.GetRotation();
	}

	inline const Vector3& Entity::GetScale() const
	{
		return m_RelativeTransform.GetScale();
	}

	inline Transform Entity::GetWorldTransform() const
	{
		return m_WorldTransformCache;
	}

	inline void Entity::SetVisible(bool bVisible)
	{
		m_bVisible = bVisible;
	}

	inline bool Entity::IsVisible() const
	{
		return m_bVisible;
	}

	inline void Entity::SetVisibleInGame(bool bVisibleInGame)
	{
		m_bVisibleInGame = bVisibleInGame;
	}

	inline bool Entity::IsVisibleInGame() const
	{
		return m_bVisibleInGame;
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
