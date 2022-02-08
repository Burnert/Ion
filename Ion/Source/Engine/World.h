#pragma once

#include "Components/Component.h"
#include "WorldTree.h"

namespace Ion
{
	class Entity;

	struct WorldInitializer
	{
		GUID WorldGuid;
	};

	class ION_API World
	{
	public:
		using EntityArray = TArray<Entity*>;

	protected:
		static World* Create(const WorldInitializer& initializer);

	public:
		void SetTickEnabled(bool bTick);

		template<typename EntityT, typename... Args>
		EntityT* SpawnEntityOfClass(Args&&... args);

		void AddEntity(Entity* entity);
		void RemoveEntity(Entity* entity);

		const WorldTree& GetWorldTree() const;

		Scene* GetScene() const;

		ComponentRegistry& GetComponentRegistry();

		/* Returns the GUID of the World.
		   A GUID is initiated at the creation of the World. */
		const GUID& GetGuid() const;

	private:
		void InitEntity(Entity* entity);

	protected:
		void BuildRendererData(RRendererData& data, float deltaTime);
		void TransferSceneRenderData();

		void OnInit();
		void OnUpdate(float deltaTime);
		void OnDestroy();

	private:
		World();
		~World();

	private:
		GUID m_WorldGUID;

		ComponentRegistry m_ComponentRegistry;
		EntityArray m_Entities; // World is the owner of the entities
		WorldTree m_WorldTree;

		Scene* m_Scene; // World is the owner of the scene

		bool m_bTickWorld;

		friend class Engine;
	};

	// Inline definitions

	template<typename EntityT, typename... Args>
	inline EntityT* World::SpawnEntityOfClass(Args&&... args)
	{
		// @TODO: Use some sort of an allocator here
		EntityT* entity = new EntityT(Forward<Args>(args)...);
		AddEntity(entity);
		return entity;
	}

	inline const WorldTree& World::GetWorldTree() const
	{
		return m_WorldTree;
	}

	inline Scene* World::GetScene() const
	{
		return m_Scene;
	}

	inline ComponentRegistry& World::GetComponentRegistry()
	{
		return m_ComponentRegistry;
	}

	inline const GUID& World::GetGuid() const
	{
		return m_WorldGUID;
	}
}
