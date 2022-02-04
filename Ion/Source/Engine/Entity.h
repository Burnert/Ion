#pragma once

namespace Ion
{
	class ION_API Entity
	{
	public:
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

		void SetTickEnabled(bool bTick);
		bool IsTickEnabled() const;

		void AddComponent(Component* component);
		void RemoveComponent(Component* component);

		void SetName(const String& name);
		const String& GetName() const;

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

	protected:
		virtual void Tick(float deltaTime) { }
		virtual void OnSpawn(World* worldContext) { }
		virtual void OnDestroy() { }

	private:
		GUID m_GUID;

		World* m_WorldContext;

		Transform m_Transform;
		THashSet<Component*> m_Components;

		bool m_bTickEnabled;

		String m_Name;

		friend class Engine;
		friend class World;
	};

	// Inline definitions

	inline void Entity::SetTransform(const Transform& transform)
	{
		m_Transform = transform;
	}

	inline const Transform& Entity::GetTransform() const
	{
		return m_Transform;
	}

	inline void Entity::SetLocation(const Vector3& location)
	{
		m_Transform.SetLocation(location);
	}

	inline const Vector3& Entity::GetLocation() const
	{
		return m_Transform.GetLocation();
	}

	inline void Entity::SetRotation(const Rotator& rotation)
	{
		m_Transform.SetRotation(rotation);
	}

	inline const Rotator& Entity::GetRotation() const
	{
		return m_Transform.GetRotation();
	}

	inline void Entity::SetScale(const Vector3& scale)
	{
		m_Transform.SetScale(scale);
	}

	inline const Vector3& Entity::GetScale() const
	{
		return m_Transform.GetScale();
	}

	inline void Entity::SetTickEnabled(bool bTick)
	{
		m_bTickEnabled = bTick;
	}

	inline bool Entity::IsTickEnabled() const
	{
		return m_bTickEnabled;
	}

	inline void Entity::SetName(const String& name)
	{
		m_Name = name;
	}

	inline const String& Entity::GetName() const
	{
		return m_Name;
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
