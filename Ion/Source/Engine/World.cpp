#include "IonPCH.h"

#include "World.h"
#include "Entity.h"
#include "Renderer/Scene.h"

#pragma warning(disable:6011)

namespace Ion
{
	World* World::Create(const WorldInitializer& initializer)
	{
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
		m_Scene = new Scene;
		m_Scene->m_OwningWorld = this;
	}

	void World::OnUpdate(float deltaTime)
	{
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
		m_ComponentRegistry.BuildRendererData(data);

		//m_Scene->UpdateRenderData();
	}

	void World::AddEntity(Entity* entity)
	{
		ionassert(entity);

		InitEntity(entity);
		m_Entities.push_back(entity);

		// @TODO: Temporary
		m_WorldTree.Add(entity);
	}

	void World::RemoveEntity(Entity* entity)
	{
		ionassert(entity);

		auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
		m_Entities.erase(it);

		m_WorldTree.Remove(entity);

		// The world owns the entity, so it should delete it.
		delete entity;
	}

	void World::InitEntity(Entity* entity)
	{
		ionassert(entity);

		entity->m_WorldContext = this;
		entity->OnSpawn(this);
	}

	World::World() :
		m_Scene(nullptr),
		m_bTickWorld(true),
		m_WorldGUID(GUID::Zero),
		m_ComponentRegistry(this)
	{
	}

	World::~World()
	{
		checked_delete(m_Scene);

		for (Entity* entity : m_Entities)
		{
			checked_delete(entity);
		}
	}

	void World::TransferSceneRenderData()
	{
		// ??
	}
}
