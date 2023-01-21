#pragma once

#include "Components/ComponentOld.h"
#include "Asset/Asset.h"
#include "Matter/ObjectPtr.h"

namespace Ion
{
#pragma region World

	class EntityOld;
	class World;

	struct WorldTreeNodeData
	{
		FORCEINLINE EntityOld* GetEntity() const
		{
			return m_Entity;
		}

		const String& GetName() const;
		const GUID& GetEntityGuid() const;

		FORCEINLINE WorldTreeNodeData(EntityOld* entity) :
			m_Entity(entity)
		{
		}

		// Root node
		FORCEINLINE WorldTreeNodeData() :
			m_Entity(nullptr)
		{
		}

	private:
		EntityOld* m_Entity;

	public:
		FORCEINLINE friend Archive& operator&=(Archive& ar, WorldTreeNodeData& data)
		{
			// Serialize the node as the entity's guid.
			if (ar.IsSaving())
			{
				GUID guid = data.GetEntityGuid();
				ar &= guid;
			}
			// NOTE: No deserialization, because the tree
			// is being rebuilt from the Entity hierarchy anyway.

			return ar;
		}
	};

	struct WorldTreeFindNodeByEntityPred
	{
		WorldTreeFindNodeByEntityPred(EntityOld* entity) : m_Entity(entity) { }

		bool operator()(WorldTreeNodeData& nodeData)
		{
			return nodeData.GetEntity() == m_Entity;
		}

	private:
		EntityOld* m_Entity;
	};

	struct WorldInitializer
	{
		GUID WorldGuid;
	};

	class ION_API World
	{
	public:
		using EntityArray = TArray<EntityOld*>;
		using EntitySet   = THashSet<EntityOld*>;
		using EntityMap   = THashMap<GUID, TObjectPtr<EntityOld>>;

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
		TObjectPtr<EntityT> SpawnEntityOfClass(Args&&... args);
		/* Instantiates an entity of type EntityT, adds it to the world and parents it to the specified entity. */
		template<typename EntityT, typename... Args>
		TObjectPtr<EntityT> SpawnAndAttachEntityOfClass(const TObjectPtr<EntityOld>& attachTo, Args&&... args);

		TObjectPtr<EntityOld> DuplicateEntity(const TObjectPtr<EntityOld>& entity);

		bool DoesOwnEntity(const TObjectPtr<EntityOld>& entity) const;
		bool DoesOwnEntity(const GUID& guid) const;
		TObjectPtr<EntityOld> FindEntity(const GUID& guid);

		void ReparentEntityInWorld(const TObjectPtr<EntityOld>& entity, const TObjectPtr<EntityOld>& parent);

		void MarkEntityForDestroy(const TObjectPtr<EntityOld>& entity);

		Scene* GetScene() const;

		WorldTreeNode& GetWorldTreeRoot();
		/* Returns nullptr if the node does not exist. */
		WorldTreeNode* FindWorldTreeNode(const TObjectPtr<EntityOld>& entity) const;
		WorldTreeNode& InsertWorldTreeNode(const TObjectPtr<EntityOld>& entity);
		WorldTreeNode& InsertWorldTreeNode(const TObjectPtr<EntityOld>& entity, WorldTreeNode& parent);
		void RemoveWorldTreeNode(const TObjectPtr<EntityOld>& entity);

		ComponentRegistry& GetComponentRegistry();

		/* Returns the GUID of the World.
		   A GUID is initiated at the creation of the World. */
		const GUID& GetGuid() const;

	private:
		void InitEntity(const TObjectPtr<EntityOld>& entity);
		void AddEntity(const TObjectPtr<EntityOld>& entity);
		void AddEntity(const TObjectPtr<EntityOld>& entity, const TObjectPtr<EntityOld>& attachTo);
		void RemoveEntity(const TObjectPtr<EntityOld>& entity);

	protected:
		void BuildRendererData(RRendererData& data, float deltaTime);
		void TransferSceneRenderData();

		void OnInit();
		void OnUpdate(float deltaTime);
		void OnDestroy();

	private:
		World();

		void AddEntityToCollection(const TObjectPtr<EntityOld>& entity);
		void RemoveEntityFromCollection(const TObjectPtr<EntityOld>& entity);

		void AddChildEntity(const TObjectPtr<EntityOld>& child);
		void RemoveChildEntity(const TObjectPtr<EntityOld>& child);

	private:
		GUID m_WorldGUID;

		ComponentRegistry m_ComponentRegistry;
		EntityArray m_EntitiesPendingKill;

		EntityMap m_Entities; // World is the owner of the entities
		EntityArray m_ChildEntities;

		WorldTreeNodeFactory m_WorldTreeNodeFactory;
		WorldTreeNode* m_WorldTreeRoot;
		THashMap<EntityOld*, WorldTreeNode*> m_EntityToWorldTreeNodeMap;

		Scene* m_Scene; // World is the owner of the scene

		bool m_bTickWorld;

		friend class Engine;

	public:
		// Serialization
		friend Archive& operator&=(Archive& ar, World* world);
	};

	// Inline definitions

	template<typename EntityT, typename... Args>
	inline TObjectPtr<EntityT> World::SpawnEntityOfClass(Args&&... args)
	{
		static_assert(TIsBaseOfV<EntityOld, EntityT>);

		// @TODO: Use some sort of an allocator here
		TObjectPtr<EntityT> entity = MObject::New<EntityT>(Forward<Args>(args)...);
		AddEntity(entity);
		return entity;
	}

	template<typename EntityT, typename ...Args>
	inline TObjectPtr<EntityT> World::SpawnAndAttachEntityOfClass(const TObjectPtr<EntityOld>& attachTo, Args&& ...args)
	{
		static_assert(TIsBaseOfV<EntityOld, EntityT>);

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
		TArray<EntityOld*> Entities;
		// @TODO: Components here vvvvv

		ASSET_DATA_GETTYPE_IMPL(AT_MapAssetType)
	};

	ASSET_TYPE_DEFAULT_DATA_INL_IMPL(MapAssetType, MapAssetData)

#pragma endregion
}
