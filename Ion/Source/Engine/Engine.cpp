#include "IonPCH.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	void Engine::Init()
	{
		TRACE_FUNCTION();
	}

	void Engine::Shutdown()
	{
		TRACE_FUNCTION();

		if (!m_RegisteredWorlds.empty())
		{
			// Copy, because DestroyWorld removes the World from the array.
			TArray<World*> worlds = m_RegisteredWorlds;

			for (World* world : worlds)
			{
				DestroyWorld(world);
			}
		}
	}

	void Engine::Update(float deltaTime)
	{
		TRACE_FUNCTION();

		// Set the global engine delta time
		m_DeltaTime = deltaTime;

		for (World* world : m_RegisteredWorlds)
		{
			world->OnUpdate(deltaTime);
		}
	}

	World* Engine::CreateWorld(const WorldInitializer& initializer)
	{
		TRACE_FUNCTION();

		World* world = World::Create(initializer);
		if (!world)
		{
			// This won't ever be reached...
			//GEngineLogger.Error("Could not create the world.");
			return nullptr;
		}

		m_RegisteredWorlds.push_back(world);
		return world;
	}

	World* Engine::CreateWorld(Archive& ar)
	{
		TRACE_FUNCTION();

		ionassert(ar.IsLoading());

		World* world = World::Create(ar);
		if (!world)
		{
			EngineLogger.Error("Could not deserialize the world from archive.");
			return nullptr;
		}

		m_RegisteredWorlds.push_back(world);
		return world;
	}

	void Engine::DestroyWorld(World* world)
	{
		TRACE_FUNCTION();

		ionassert(world);

		auto it = std::find(m_RegisteredWorlds.begin(), m_RegisteredWorlds.end(), world);
		m_RegisteredWorlds.erase(it);

		world->OnDestroy();
		delete world;
	}

	World* Engine::FindWorld(const GUID& worldGuid) const
	{
		auto it = std::find_if(m_RegisteredWorlds.begin(), m_RegisteredWorlds.end(), [&worldGuid](World* world)
		{
			return world->GetGuid() == worldGuid;
		});
		if (it == m_RegisteredWorlds.end())
			return nullptr;

		return *it;
	}

	void Engine::BuildRendererData(float deltaTime)
	{
		TRACE_FUNCTION();

		for (World* world : m_RegisteredWorlds)
		{
			Scene* scene = world->GetScene();
			if (!scene)
				continue;

			RRendererData data { };
			world->BuildRendererData(data, deltaTime);

			scene->LoadSceneData(data);
		}
	}

	Engine* g_Engine = new Engine;
}
