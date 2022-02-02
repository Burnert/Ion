#include "IonPCH.h"

#include "Engine.h"

namespace Ion
{
	void Engine::Init()
	{
	}

	void Engine::Shutdown()
	{
	}

	void Engine::Update(float deltaTime)
	{
		// Set the global engine delta time
		m_DeltaTime = deltaTime;

		for (World* world : m_RegisteredWorlds)
		{
			world->Update(deltaTime);
		}
	}

	void Engine::RegisterWorld(World* world)
	{
		ionassert(world);
		ionassert(std::find(m_RegisteredWorlds.begin(), m_RegisteredWorlds.end(), world) == m_RegisteredWorlds.end());

		m_RegisteredWorlds.push_back(world);
	}

	void Engine::UnregisterWorld(World* world)
	{
		ionassert(world);

		auto it = std::find(m_RegisteredWorlds.begin(), m_RegisteredWorlds.end(), world);
		m_RegisteredWorlds.erase(it);
	}

	void Engine::Update_SyncRenderingData(float deltaTime)
	{
		for (World* world : m_RegisteredWorlds)
		{
			world->Update_SyncRenderingData(deltaTime);
		}
	}

	Engine* g_Engine = new Engine;
}
