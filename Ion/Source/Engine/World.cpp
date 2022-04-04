#include "IonPCH.h"

#include "World.h"
#include "Entity/Entity.h"
#include "Renderer/Scene.h"

#pragma warning(disable:6011)

namespace Ion
{
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

	void World::OnInit()
	{
		TRACE_FUNCTION();

		m_Scene = new Scene;
		m_Scene->m_OwningWorld = this;
	}

	void World::OnUpdate(float deltaTime)
	{
		TRACE_FUNCTION();

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

	void World::AddEntity(Entity* entity)
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

	void World::AddEntity(Entity* entity, Entity* attachTo)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(attachTo);
		ionassert(!DoesOwnEntity(entity), "Entity already exists in the world.");

		WorldTreeNode* parentNode = FindWorldTreeNode(attachTo);
		ionassertnd(parentNode, "Entity to attach to doesn't exist in the world.");

		Entity* parent = parentNode->Get().AsEntity();

		InitEntity(entity);

		AddEntityToCollection(entity);
		// Attach the entity to the parent node
		InsertWorldTreeNode(entity, *parentNode);

		// Update entities
		parent->AddChild(entity);
		entity->m_Parent = parent;
	}

	void World::RemoveEntity(Entity* entity)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(DoesOwnEntity(entity), "Entity doesn't exist in the world.");

		entity->OnDestroy();

		RemoveEntityFromCollection(entity);
		RemoveChildEntity(entity);

		RemoveWorldTreeNode(entity);

		// The world owns the entity, so it should delete it.
		delete entity;
	}

	bool World::DoesOwnEntity(Entity* entity) const
	{
		return DoesOwnEntity(entity->GetGuid());
	}

	bool World::DoesOwnEntity(const GUID& guid) const
	{
		return m_Entities.find(guid) != m_Entities.end();
	}

	void World::ReparentEntityInWorld(Entity* entity, Entity* parent)
	{
		TRACE_FUNCTION();

		ionassert(entity);

		// Find the nodes
		WorldTreeNode* entityNode = FindWorldTreeNode(entity);
		ionassertnd(entityNode, "The entity is not in the world tree.");

		WorldTreeNode* parentNode = &m_WorldTreeRoot;
		if (parent)
		{
			parentNode = FindWorldTreeNode(parent);
			ionassertnd(parentNode, "The parent entity is not in the world tree.");

			RemoveChildEntity(entity);
		}
		else
		{
			AddChildEntity(entity);
		}

		// Reparent in world tree
		parentNode->Insert(entityNode->RemoveFromParent());
	}

	Entity* World::FindEntity(const GUID& guid)
	{
		auto it = m_Entities.find(guid);
		if (it == m_Entities.end())
			return nullptr;

		return it->second;
	}

	World::WorldTreeNode* World::FindWorldTreeNode(Entity* entity) const
	{
		auto it = m_EntityToWorldTreeNodeMap.find(entity);
		if (it != m_EntityToWorldTreeNodeMap.end())
		{
			return it->second;
		}
		return nullptr;
	}

	World::WorldTreeNode& World::InsertWorldTreeNode(Entity* entity)
	{
		return InsertWorldTreeNode(entity, m_WorldTreeRoot);
	}

	World::WorldTreeNode& World::InsertWorldTreeNode(Entity* entity, WorldTreeNode& parent)
	{
		TRACE_FUNCTION();

		ionassert(!FindWorldTreeNode(entity), "The node for that entity already exists.");

		WorldTreeNode& node = parent.Insert(m_WorldTreeNodeFactory.Create(WorldTreeNodeData(entity)));
		m_EntityToWorldTreeNodeMap.insert({entity, &node});
		return node;
	}

	void World::RemoveWorldTreeNode(Entity* entity)
	{
		TRACE_FUNCTION();

		WorldTreeNode* node = FindWorldTreeNode(entity);
		ionassertnd(node, "The entity was not in the world tree.");

		m_WorldTreeNodeFactory.Destroy(node->RemoveFromParent());
	}

	inline World::WorldTreeNode& World::GetWorldTreeRoot()
	{
		return m_WorldTreeRoot;
	}

	void World::InitEntity(Entity* entity)
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
		m_WorldTreeRoot(m_WorldTreeNodeFactory.Create(WorldTreeNodeData()))
	{
	}

	World::~World()
	{
		TRACE_FUNCTION();

		checked_delete(m_Scene);

		for (auto& [guid, entity] : m_Entities)
		{
			checked_delete(entity);
		}
		m_Entities.clear();
	}

	void World::AddEntityToCollection(Entity* entity)
	{
		m_Entities.insert({ entity->GetGuid(), entity });
	}

	void World::RemoveEntityFromCollection(Entity* entity)
	{
		m_Entities.erase(entity->GetGuid());
	}

	void World::AddChildEntity(Entity* child)
	{
		m_ChildEntities.push_back(child);
	}

	void World::RemoveChildEntity(Entity* child)
	{
		auto it = std::find(m_ChildEntities.begin(), m_ChildEntities.end(), child);
		if (it != m_ChildEntities.end())
			m_ChildEntities.erase(it);
	}

	void World::TransferSceneRenderData()
	{
		// ??
	}
}
