#include "IonPCH.h"

#include "World.h"
#include "Entity.h"
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

		for (Entity* entity : m_Entities)
		{
			entity->Update(deltaTime);
		}

		m_ComponentRegistry.Update(deltaTime);
	}

	void World::OnDestroy()
	{
		TRACE_FUNCTION();

		for (Entity* entity : m_Entities)
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

		//m_Scene->UpdateRenderData();
	}

	void World::AddEntity(Entity* entity)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(m_Entities.find(entity) == m_Entities.end(), "Entity already exists in the world.");

		InitEntity(entity);

		m_Entities.insert(entity);
		AddChildEntity(entity);

		m_WorldTreeRoot.Insert(m_WorldTreeNodeFactory.Create(WorldTreeNodeData(entity)));
	}

	void World::RemoveEntity(Entity* entity)
	{
		TRACE_FUNCTION();

		ionassert(entity);
		ionassert(m_Entities.find(entity) != m_Entities.end(), "Entity doesn't exist in the world.");

		m_Entities.erase(entity);
		RemoveChildEntity(entity);

		WorldTreeNode* node = m_WorldTreeRoot.FindNodeRecursiveDF(WorldTreeFindNodeByEntityPred(entity));
		ionassertnd(node, "The entity was not in the world tree.");

		m_WorldTreeNodeFactory.Destroy(node->RemoveFromParent());

		// The world owns the entity, so it should delete it.
		delete entity;
	}

	void World::ReparentEntityInWorld(Entity* entity, Entity* parent)
	{
		ionassert(entity);

		// Find the nodes
		WorldTreeNode* entityNode = m_WorldTreeRoot.FindNodeRecursiveDF(WorldTreeFindNodeByEntityPred(entity));
		ionassertnd(entityNode, "The entity is not in the world tree.");

		WorldTreeNode* parentNode = &m_WorldTreeRoot;
		if (parent)
		{
			parentNode = m_WorldTreeRoot.FindNodeRecursiveDF(WorldTreeFindNodeByEntityPred(parent));
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

		for (Entity* entity : m_Entities)
		{
			checked_delete(entity);
		}
		m_Entities.clear();
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
