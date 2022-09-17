#pragma once

#include "Components/Component.h"
#include "Asset/Asset.h"

namespace Ion
{
#pragma region World

	class Entity;
	class World;

	struct WorldTreeNodeData
	{
		FORCEINLINE Entity* GetEntity() const
		{
			return m_Entity;
		}

		const String& GetName() const;
		const GUID& GetEntityGuid() const;

		FORCEINLINE WorldTreeNodeData(Entity* entity) :
			m_Entity(entity)
		{
		}

		// Root node
		FORCEINLINE WorldTreeNodeData() :
			m_Entity(nullptr)
		{
		}

	private:
		Entity* m_Entity;

	public:
		FORCEINLINE friend Archive& operator<<(Archive& ar, WorldTreeNodeData& data)
		{
			// Serialize the node as the entity's guid.
			if (ar.IsSaving())
			{
				GUID guid = data.GetEntityGuid();
				ar << guid;
			}
			// NOTE: No deserialization, because the tree
			// is being rebuilt from the Entity hierarchy anyway.

			return ar;
		}
	};

	struct WorldTreeFindNodeByEntityPred
	{
		WorldTreeFindNodeByEntityPred(Entity* entity) : m_Entity(entity) { }

		bool operator()(WorldTreeNodeData& nodeData)
		{
			return nodeData.GetEntity() == m_Entity;
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
		using WorldTreeNode        = TFastTreeNode<WorldTreeNodeData>;

	protected:
		static World* Create(const WorldInitializer& initializer);
		static World* Create(Archive& ar);

	public:
		static World* LoadFromAsset(const Asset& mapAsset);
		void SaveToAsset(const Asset& mapAsset);

		void SetTickEnabled(bool bTick);

		/* Instantiates an entity of type EntityT and adds it to the world. */
		template<typename EntityT, typename... Args>
		EntityT* SpawnEntityOfClass(Args&&... args);
		/* Instantiates an entity of type EntityT, adds it to the world and parents it to the specified entity. */
		template<typename EntityT, typename... Args>
		EntityT* SpawnAndAttachEntityOfClass(Entity* attachTo, Args&&... args);

		Entity* DuplicateEntity(Entity* entity);

		bool DoesOwnEntity(Entity* entity) const;
		bool DoesOwnEntity(const GUID& guid) const;
		Entity* FindEntity(const GUID& guid);

		void ReparentEntityInWorld(Entity* entity, Entity* parent);

		void MarkEntityForDestroy(Entity* entity);

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
		void AddEntity(Entity* entity);
		void AddEntity(Entity* entity, Entity* attachTo);
		void RemoveEntity(Entity* entity);

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
		EntityArray m_EntitiesPendingKill;

		EntityMap m_Entities; // World is the owner of the entities
		EntityArray m_ChildEntities;

		WorldTreeNodeFactory m_WorldTreeNodeFactory;
		WorldTreeNode* m_WorldTreeRoot;
		THashMap<Entity*, WorldTreeNode*> m_EntityToWorldTreeNodeMap;

		Scene* m_Scene; // World is the owner of the scene

		bool m_bTickWorld;

		friend class Engine;

	public:
		// Serialization
		friend Archive& operator<<(Archive& ar, World* world);
	};

	// Inline definitions

	template<typename EntityT, typename... Args>
	inline EntityT* World::SpawnEntityOfClass(Args&&... args)
	{
		static_assert(TIsBaseOfV<Entity, EntityT>);

		// @TODO: Use some sort of an allocator here
		EntityT* entity = MObject::New<EntityT>(Forward<Args>(args)...).Raw();
		AddEntity(entity);
		return entity;
	}

	template<typename EntityT, typename ...Args>
	inline EntityT* World::SpawnAndAttachEntityOfClass(Entity* attachTo, Args&& ...args)
	{
		static_assert(TIsBaseOfV<Entity, EntityT>);

		EntityT* entity = MObject::New<EntityT>(Forward<Args>(args)...);
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

#pragma endregion

#pragma region Map Asset / Serialization

	class MapAssetType : public IAssetType
	{
	public:
		virtual Result<void, IOError> Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const override;
		virtual TSharedPtr<IAssetCustomData> CreateDefaultCustomData() const override;
		ASSET_TYPE_NAME_IMPL("Ion.Map")
	};

	REGISTER_ASSET_TYPE_CLASS(MapAssetType);

	class MapAssetData : public IAssetCustomData
	{
	public:
		GUID WorldGuid = GUID::Zero;
		TArray<Entity*> Entities;
		// @TODO: Components here vvvvv

		ASSET_DATA_GETTYPE_IMPL(AT_MapAssetType)
	};

	ASSET_TYPE_DEFAULT_DATA_INL_IMPL(MapAssetType, MapAssetData)

#pragma endregion
}
