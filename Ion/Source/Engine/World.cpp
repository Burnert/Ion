#include "IonPCH.h"

#include "World.h"
#include "Entity.h"
#include "Renderer/Scene.h"

namespace Ion
{
	World* World::CreateWorld(const WorldInitializer& initializer)
	{
		// @TODO: Who should own the world?
		// Worlds have to be deleted at some point
		World* world = new World;

		// @TODO: initializer stuff
		if (initializer.WorldGuid)
		{
			world->m_WorldGUID = initializer.WorldGuid;
		}

		world->Init();

		return world;
	}

	void World::Init()
	{
		m_Scene = new Scene;
		m_Scene->m_OwningWorld = this;
	}

	void World::Update(float deltaTime)
	{
		if (!m_bTickWorld)
			return;

		for (Entity* entity : m_Entities)
		{
			entity->Update(deltaTime);
		}

		m_ComponentRegistry.Update(deltaTime);
	}

	void World::SetTickEnabled(bool bTick)
	{
		m_bTickWorld = bTick;
	}

	void World::Update_SyncRenderingData(float deltaTime)
	{
		m_Scene->UpdateRenderData();
	}

	void World::AddEntity(Entity* entity)
	{
		ionassert(entity);

		InitEntity(entity);
		m_Entities.push_back(entity);
	}

	void World::RemoveEntity(Entity* entity)
	{
		ionassert(entity);

		EntityArray::iterator it = std::find(m_Entities.begin(), m_Entities.end(), entity);
		Entity* ptr = *it;
		// The world owns the entity, so it should delete it.
		delete ptr;
		m_Entities.erase(it);
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
			ionassert(entity);

			if (entity->m_WorldContext == this)
				delete entity;
		}
	}

	void World::TransferSceneRenderData()
	{
		// ??
	}
}
