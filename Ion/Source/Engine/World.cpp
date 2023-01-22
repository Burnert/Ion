#include "IonPCH.h"

#include "World.h"
#include "Entity/EntityOld.h"
#include "Entity/Entity.h"
#include "Renderer/Scene.h"
#include "Asset/AssetDefinition.h"

#pragma warning(disable:6011)

namespace Ion
{
	const String& WorldTreeNodeData::GetName() const
	{
		return m_Entity ? m_Entity->GetName() : EmptyString;
	}

	const GUID& WorldTreeNodeData::GetEntityGuid() const
	{
		return m_Entity ? m_Entity->GetGuid() : GUID::Zero;
	}

	World* World::Create(const WorldInitializer& initializer)
	{
		TRACE_FUNCTION();

		// @TODO: Who should own the world?
		// Worlds have to be deleted at some point
		World* world = new World;

		// @TODO: initializer stuff
		if (initializer.WorldGuid)
		{
			world->m_WorldGUID = initializer.WorldGuid;
		}

		world->OnInit();

		return world;
	}

	World* World::Create(Archive& ar)
	{
		TRACE_FUNCTION();

		ionassert(ar.IsLoading());

		// @TODO: Who should own the world?
		// Worlds have to be deleted at some point
		World* world = new World;
		ar &= world;
		world->OnInit();

		return world;
	}

	World* World::LoadFromAsset(const Asset& mapAsset)
	{
		ionassert(mapAsset->GetType() == AT_MapAssetType);

		TSharedPtr<MapAssetData> mapData = PtrCast<MapAssetData>(mapAsset->GetCustomData());

		World* world = new World;

		world->m_WorldGUID = mapData->WorldGuid;

		for (EntityOld* entity : mapData->Entities)
		{
			world->AddEntity(entity->This());
		}

		// @TODO: setup relations

		world->OnInit();

		return world;
	}

	void World::SaveToAsset(const Asset& mapAsset)
	{
		ionassert(mapAsset->GetType() == AT_MapAssetType);

		TSharedPtr<MapAssetData> mapData = PtrCast<MapAssetData>(mapAsset->GetCustomData());

		mapData->WorldGuid = m_WorldGUID;
		mapData->Entities = [&]
		{
			TArray<TObjectPtr<EntityOld>> entities = GatherValues(m_Entities);
			TArray<EntityOld*> rawPtrs;
			rawPtrs.reserve(entities.size());
			for (const TObjectPtr<EntityOld>& entity : entities)
			{
				rawPtrs.emplace_back(entity.Raw());
			}
			return rawPtrs;
		}();

		mapAsset->SaveToDisk();
	}

	void World::OnInit()
	{
		TRACE_FUNCTION();

		m_Scene = new Scene;
		m_Scene->m_OwningWorld = this;
	}

	void World::OnUpdate(float deltaTime)
	{
		TRACE_FUNCTION();

		m_ComponentRegistry.DestroyInvalidComponents();
		// Destroy entities that are pending kill
		for (EntityOld* entity : m_EntitiesPendingKill)
		{
			RemoveEntity(entity->This());
		}
		m_EntitiesPendingKill.clear();

		if (!m_bTickWorld)
			return;

		for (auto& [guid, entity] : m_Entities)
		{
			entity->Update(deltaTime);
		}

		m_ComponentRegistry.Update(deltaTime);
	}

	void World::OnDestroy()
	{
		TRACE_FUNCTION();

		for (auto& [guid, entity] : m_Entities)
		{
			entity->OnDestroy();
		}
		m_Entities.clear();

		checked_delete(m_Scene);
	}

	void World::SetTickEnabled(bool bTick)
	{
		m_bTickWorld = bTick;
	}

	void World::BuildRendererData(RRendererData& data, float deltaTime)
	{
		TRACE_FUNCTION();

		m_ComponentRegistry.BuildRendererData(data);
	}

	void World::AddEntity(const TObjectPtr<EntityOld>& entity)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(!DoesOwnEntity(entity), "Entity already exists in the world.");

		InitEntity(entity);

		// Insert the entity to collections
		AddEntityToCollection(entity);
		AddChildEntity(entity);
		InsertWorldTreeNode(entity);
	}

	void World::AddEntity(const TObjectPtr<EntityOld>& entity, const TObjectPtr<EntityOld>& attachTo)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(attachTo);
		ionassert(!DoesOwnEntity(entity), "Entity already exists in the world.");

		WorldTreeNode* parentNode = FindWorldTreeNode(attachTo);
		ionverify(parentNode, "Entity to attach to doesn't exist in the world.");

		TObjectPtr<EntityOld> parent = parentNode->Get().GetEntity()->This();

		InitEntity(entity);

		AddEntityToCollection(entity);
		// Attach the entity to the parent node
		InsertWorldTreeNode(entity, *parentNode);

		// Update entities
		parent->AddChild(entity);
		entity->m_Parent = parent;
	}

	void World::RemoveEntity(const TObjectPtr<EntityOld>& entity)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(DoesOwnEntity(entity), "Entity doesn't exist in the world.");

		entity->OnDestroy();

		RemoveEntityFromCollection(entity);
		RemoveChildEntity(entity);

		RemoveWorldTreeNode(entity);

		// The entity lifetime is now managed by Matter
		
		// The world owns the entity, so it should delete it.
		//delete entity;
	}

	TObjectPtr<EntityOld> World::DuplicateEntity(const TObjectPtr<EntityOld>& entity)
	{
		if (entity->IsPendingKill())
			return nullptr;

		EntityOld* newEntity = entity->Duplicate();

		// @TODO: Remove this nonsense

		//AddEntityToCollection(newEntity);

		//if (entity->HasParent())
		//{
		//	WorldTreeNode* parentNode = FindWorldTreeNode(entity->GetParent());
		//	ionverify(parentNode, "Entity to attach to doesn't exist in the world.");

		//	// Attach the entity to the parent node
		//	InsertWorldTreeNode(newEntity, *parentNode);

		//	// Update entities
		//	entity->GetParent()->AddChild(newEntity);
		//}
		//else
		//{
		//	InsertWorldTreeNode(newEntity);
		//	AddChildEntity(newEntity);
		//}

		return nullptr;
	}

	bool World::DoesOwnEntity(const TObjectPtr<EntityOld>& entity) const
	{
		return DoesOwnEntity(entity->GetGuid());
	}

	bool World::DoesOwnEntity(const GUID& guid) const
	{
		return m_Entities.find(guid) != m_Entities.end();
	}

	void World::ReparentEntityInWorld(const TObjectPtr<EntityOld>& entity, const TObjectPtr<EntityOld>& parent)
	{
		TRACE_FUNCTION();

		ionassert(entity);

		// Find the nodes
		WorldTreeNode* entityNode = FindWorldTreeNode(entity);
		ionverify(entityNode, "The entity is not in the world tree.");

		WorldTreeNode* parentNode = m_WorldTreeRoot;
		if (parent)
		{
			parentNode = FindWorldTreeNode(parent);
			ionverify(parentNode, "The parent entity is not in the world tree.");

			RemoveChildEntity(entity);
		}
		else
		{
			AddChildEntity(entity);
		}

		// Reparent in world tree
		parentNode->Insert(entityNode->RemoveFromParent());
	}

	void World::MarkEntityForDestroy(const TObjectPtr<EntityOld>& entity)
	{
		m_EntitiesPendingKill.push_back(entity.Raw());
	}

	TObjectPtr<EntityOld> World::FindEntity(const GUID& guid)
	{
		auto it = m_Entities.find(guid);
		if (it == m_Entities.end())
			return nullptr;

		return it->second;
	}

	World::WorldTreeNode* World::FindWorldTreeNode(const TObjectPtr<EntityOld>& entity) const
	{
		auto it = m_EntityToWorldTreeNodeMap.find(entity.Raw());
		if (it != m_EntityToWorldTreeNodeMap.end())
		{
			return it->second;
		}
		return nullptr;
	}

	World::WorldTreeNode& World::InsertWorldTreeNode(const TObjectPtr<EntityOld>& entity)
	{
		return InsertWorldTreeNode(entity, *m_WorldTreeRoot);
	}

	World::WorldTreeNode& World::InsertWorldTreeNode(const TObjectPtr<EntityOld>& entity, WorldTreeNode& parent)
	{
		TRACE_FUNCTION();

		ionassert(!FindWorldTreeNode(entity), "The node for that entity already exists.");

		WorldTreeNode& node = parent.Insert(m_WorldTreeNodeFactory.Create(WorldTreeNodeData(entity.Raw())));
		m_EntityToWorldTreeNodeMap.insert({ entity.Raw(), &node });
		return node;
	}

	void World::RemoveWorldTreeNode(const TObjectPtr<EntityOld>& entity)
	{
		TRACE_FUNCTION();

		WorldTreeNode* node = FindWorldTreeNode(entity);
		ionverify(node, "The entity was not in the world tree.");

		m_WorldTreeNodeFactory.Destroy(node->RemoveFromParent());
		m_EntityToWorldTreeNodeMap.erase(entity.Raw());
	}

	World::WorldTreeNode& World::GetWorldTreeRoot()
	{
		return *m_WorldTreeRoot;
	}

	void World::InitEntity(const TObjectPtr<EntityOld>& entity)
	{
		TRACE_FUNCTION();

		ionassert(entity);

		entity->m_WorldContext = this;
		entity->OnSpawn(this);
	}

	World::World() :
		m_Scene(nullptr),
		m_bTickWorld(true),
		m_WorldGUID(GUID::Zero),
		m_ComponentRegistry(this),
		m_WorldTreeRoot(&m_WorldTreeNodeFactory.Create(WorldTreeNodeData()))
	{
	}

	void World::AddEntityToCollection(const TObjectPtr<EntityOld>& entity)
	{
		ionassert(m_Entities.find(entity->GetGuid()) == m_Entities.end());
		m_Entities.insert({ entity->GetGuid(), entity });
	}

	void World::RemoveEntityFromCollection(const TObjectPtr<EntityOld>& entity)
	{
		m_Entities.erase(entity->GetGuid());
	}

	void World::AddChildEntity(const TObjectPtr<EntityOld>& child)
	{
		ionassert(std::find(m_ChildEntities.begin(), m_ChildEntities.end(), child.Raw()) == m_ChildEntities.end());
		m_ChildEntities.push_back(child.Raw());
	}

	void World::RemoveChildEntity(const TObjectPtr<EntityOld>& child)
	{
		auto it = std::find(m_ChildEntities.begin(), m_ChildEntities.end(), child.Raw());
		if (it != m_ChildEntities.end())
			m_ChildEntities.erase(it);
	}

	void World::TransferSceneRenderData()
	{
		// ??
	}

	Archive& operator&=(Archive& ar, World* world)
	{
		ionbreak("Not working.");
		ionassert(world);

		ar &= world->m_WorldGUID;

		//ar &= &world->m_ComponentRegistry;

		TArray<TObjectPtr<EntityOld>> allEntities = ar.IsSaving() ? GatherValues(world->m_Entities) : TArray<TObjectPtr<EntityOld>>();
		//ar &= allEntities;
		if (ar.IsLoading())
		{
			for (const TObjectPtr<EntityOld>& entity : allEntities)
			{
				world->m_Entities.emplace(entity->GetGuid(), entity);
			}
		}
		
		ar &= TTreeSerializer(*world->m_WorldTreeRoot, world->m_WorldTreeNodeFactory);

		if (ar.IsLoading())
		{
			// Fill in the fields which are not saved by the serializer.

			TFunction<void(World::WorldTreeNode*)> LLoadWorldTreeNode = [&](World::WorldTreeNode* node)
			{
				ionassert(node);

				if (EntityOld* entity = node->Get().GetEntity())
				{
					world->m_EntityToWorldTreeNodeMap.emplace(entity, node);

					world->m_Entities.emplace(entity->GetGuid(), entity);
				}

				for (World::WorldTreeNode* n : node->GetChildren())
				{
					LLoadWorldTreeNode(n);
				}
			};

			LLoadWorldTreeNode(world->m_WorldTreeRoot);

			world->m_ChildEntities.reserve(world->m_WorldTreeRoot->GetChildren().size());
			for (World::WorldTreeNode* worldChildNode : world->m_WorldTreeRoot->GetChildren())
			{
				if (EntityOld* entity = worldChildNode->Get().GetEntity())
				{
					world->m_ChildEntities.emplace_back(entity);
				}
			}
		}

		ar &= world->m_bTickWorld;

		return ar;
	}

#pragma region Map Asset

#define IASSET_NODE_World    "World"
#define IASSET_NODE_Entity   "Entity"
#define IASSET_NODE_Entities "Entities"

	Result<void, IOError> MapAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");
		ionassert(!inOutCustomData || inOutCustomData->GetType() == AT_MapAssetType);

		TSharedPtr<MapAssetData> data = inOutCustomData ? PtrCast<MapAssetData>(inOutCustomData) : MakeShared<MapAssetData>();

		XMLArchiveAdapter xmlAr = ar;

		xmlAr.EnterNode(IASSET_NODE_World);

		xmlAr.EnterNode(IASSET_NODE_Guid);
		xmlAr &= data->WorldGuid;
		xmlAr.ExitNode(); // IASSET_NODE_Guid

		xmlAr.EnterNode(IASSET_NODE_Entities);
		auto LSerializeEntity = [&](int32 index = -1)
		{
			if (ar.IsSaving())
				;// ar &= SerializeMObject(data->Entities[index]);
			else if (ar.IsLoading())
				;// ar &= SerializeMObject(data->Entities.emplace_back());
		};
		if (ar.IsLoading())
		{
			for (bool b = xmlAr.TryEnterNode(IASSET_NODE_Entity);
				b || (xmlAr.ExitNode(), 0);
				b = xmlAr.TryEnterSiblingNode())
				LSerializeEntity();
		}
		else if (ar.IsSaving() && !data->Entities.empty())
		{
			for (int32 i = 0; i < data->Entities.size(); ++i)
			{
				xmlAr.EnterNode(IASSET_NODE_Entity);
				LSerializeEntity(i);
				xmlAr.ExitNode(); // IASSET_NODE_Entity
			}
		}
		xmlAr.ExitNode(); // IASSET_NODE_Entities

		xmlAr.ExitNode(); // IASSET_NODE_World

		inOutCustomData = data;

		return Ok();
	}

#pragma endregion

#pragma region MWorld

	MWorld::MWorld() :
		m_Scene(nullptr)
	{
	}

	void MWorld::OnCreate()
	{
		TRACE_FUNCTION();

		m_Scene = new Scene;
	}
	
	void MWorld::OnDestroy()
	{
		checked_delete(m_Scene);
	}
	
	void MWorld::Tick(float deltaTime)
	{
	}

	void MWorld::BuildRendererData(RRendererData& data)
	{
		TArray<TObjectPtr<MSceneComponent>> visibleComponents = GatherVisibleSceneComponents();

		for (const TObjectPtr<MSceneComponent>& component : visibleComponents)
		{
			// component->BuildRendererData(data);
		}
	}

	TArray<TObjectPtr<MSceneComponent>> MWorld::GatherVisibleSceneComponents() const
	{
		TArray<TObjectPtr<MSceneComponent>> components;
		// It will probably not be enough if the entities have multiple scene components visible.
		components.reserve(m_Entities.size());
		
		for (auto& [guid, entity] : m_Entities)
		{
			// @TODO: Gather all components
			const TObjectPtr<MSceneComponent>& root = entity->GetRootComponent();
			if (root && root->IsVisible())
			{
				components.emplace_back(root);
			}
		}

		return components;
	}

	void MWorld::AddEntity(const TObjectPtr<MEntity>& entity)
	{
		ionassert(m_Entities.find(entity->GetGuid()) == m_Entities.end(), "Entity {} is already owned by world {}.", entity->GetName(), GetName());

		m_Entities.emplace(entity->GetGuid(), entity);
		entity->OnSpawn();
	}

#pragma endregion
}
