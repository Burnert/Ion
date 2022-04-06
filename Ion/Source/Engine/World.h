#pragma once

#include "Components/Component.h"

namespace Ion
{
	class Entity;
	class World;

	struct WorldTreeFolder
	{
		String Name;
	};

	struct WorldTreeNodeData
	{
		inline Entity* AsEntity() const
		{
			return (Entity*)m_Pointer.Get();
		}

		inline WorldTreeFolder* AsFolder() const
		{
			return (WorldTreeFolder*)m_Pointer.Get();
		}

		inline bool IsFolder() const
		{
			return m_Pointer.GetMetaFlag<0>();
		}

		inline WorldTreeNodeData(Entity* entity) :
			m_Pointer((uint8*)entity)
		{
			m_Pointer.SetMetaFlag<0>(false);
		}

		inline WorldTreeNodeData(WorldTreeFolder* folder) :
			m_Pointer((uint8*)folder)
		{
			m_Pointer.SetMetaFlag<0>(true);
		}

		// Root node
		inline WorldTreeNodeData() :
			m_Pointer(nullptr)
		{
			m_Pointer.SetMetaFlag<0>(true);
		}

	private:
		TMetaPointer<uint8> m_Pointer;
	};

	struct WorldTreeFindNodeByEntityPred
	{
		WorldTreeFindNodeByEntityPred(Entity* entity) : m_Entity(entity) { }

		bool operator()(WorldTreeNodeData& nodeData)
		{
			if (nodeData.IsFolder())
				return false;
			return nodeData.AsEntity() == m_Entity;
		}

	private:
		Entity* m_Entity;
	};

	struct WorldInitializer
	{
		GUID WorldGuid;
	};

	class ION_API World
	{
	public:
		using EntityArray = TArray<Entity*>;
		using EntitySet   = THashSet<Entity*>;
		using EntityMap   = THashMap<GUID, Entity*>;

		using WorldTreeNodeFactory = TTreeNodeFactory<WorldTreeNodeData>;
		using WorldTreeNode        = TTreeNode<WorldTreeNodeData>;

	protected:
		static World* Create(const WorldInitializer& initializer);

	public:
		void SetTickEnabled(bool bTick);

		/* Instantiates an entity of type EntityT and adds it to the world. */
		template<typename EntityT, typename... Args>
		EntityT* SpawnEntityOfClass(Args&&... args);
		/* Instantiates an entity of type EntityT, adds it to the world and parents it to the specified entity. */
		template<typename EntityT, typename... Args>
		EntityT* SpawnAndAttachEntityOfClass(Entity* attachTo, Args&&... args);

		void AddEntity(Entity* entity);
		void AddEntity(Entity* entity, Entity* attachTo);
		void RemoveEntity(Entity* entity);

		bool DoesOwnEntity(Entity* entity) const;
		bool DoesOwnEntity(const GUID& guid) const;
		Entity* FindEntity(const GUID& guid);

		void ReparentEntityInWorld(Entity* entity, Entity* parent);

		Scene* GetScene() const;

		WorldTreeNode& GetWorldTreeRoot();
		/* Returns nullptr if the node does not exist. */
		WorldTreeNode* FindWorldTreeNode(Entity* entity) const;
		WorldTreeNode& InsertWorldTreeNode(Entity* entity);
		WorldTreeNode& InsertWorldTreeNode(Entity* entity, WorldTreeNode& parent);
		void RemoveWorldTreeNode(Entity* entity);

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

		void AddEntityToCollection(Entity* entity);
		void RemoveEntityFromCollection(Entity* entity);

		void AddChildEntity(Entity* child);
		void RemoveChildEntity(Entity* child);

	private:
		GUID m_WorldGUID;

		ComponentRegistry m_ComponentRegistry;

		EntityMap m_Entities; // World is the owner of the entities
		EntityArray m_ChildEntities;

		WorldTreeNodeFactory m_WorldTreeNodeFactory;
		WorldTreeNode m_WorldTreeRoot;
		THashMap<Entity*, WorldTreeNode*> m_EntityToWorldTreeNodeMap;

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

	template<typename EntityT, typename ...Args>
	inline EntityT* World::SpawnAndAttachEntityOfClass(Entity* attachTo, Args&& ...args)
	{
		EntityT* entity = new EntityT(Forward<Args>(args)...);
		AddEntity(entity, attachTo);
		return entity;
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
