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
			LOG_ERROR("Could not create the world.");
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

	void Engine::BuildRendererData(float deltaTime)
	{
		TRACE_FUNCTION();

		for (World* world : m_RegisteredWorlds)
		{
			Scene* scene = world->GetScene();
			if (!scene) continue;

			RRendererData data { };
			world->BuildRendererData(data, deltaTime);

			scene->LoadSceneData(data);
		}
	}

	Engine* g_Engine = new Engine;
}
