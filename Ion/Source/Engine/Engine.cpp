#include "IonPCH.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

#define EXIT_ON_UNREGISTERED_MOBJECT(object) if (m_MObjects.find(object->GetGuid()) == m_MObjects.end()) return

namespace Ion
{
	void EngineMObjectInterface::RegisterObject(const MObjectPtr& object)
	{
		ionassert(g_Engine, "This shouldn't ever be called before the engine is initialized.");
		g_Engine->RegisterObject(object);
	}

	void EngineMObjectInterface::SetObjectTickEnabled(const MObjectPtr& object, bool bEnabled)
	{
		if (g_Engine)
			g_Engine->SetObjectTickEnabled(object, bEnabled);
	}

	Engine::Engine() :
		m_DeltaTime(0.016666666f)
	{
	}

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

		RemoveInvalidObjectPointers();

		for (World* world : m_RegisteredWorlds)
		{
			world->OnUpdate(deltaTime);
		}

		for (auto& [guid, weakObject] : m_TickingMObjects)
		{
			ionassert(!weakObject.IsExpired(), "MObject with guid {} has expired and did not get deleted before ticking.", guid);

			MObject* object = weakObject.Raw();
			object->Tick(deltaTime);
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

	World* Engine::CreateWorldFromMapAsset(const Asset& mapAsset)
	{
		TRACE_FUNCTION();

		World* world = World::LoadFromAsset(mapAsset);
		if (!world)
		{
			// This won't ever be reached...
			//GEngineLogger.Error("Could not create the world.");
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

		for (auto& [guid, world] : m_ActiveWorlds)
		{
			if (!world->GetScene())
				continue;

			RRendererData data { };
			world->BuildRendererData(data);

			world->GetScene()->LoadSceneData(data);
		}
	}

	void Engine::AddWorld(const TObjectPtr<MWorld>& world)
	{
		ionassert(m_ActiveWorlds.find(world->GetGuid()) == m_ActiveWorlds.end());

		m_ActiveWorlds.emplace(world->GetGuid(), world);
	}

	void Engine::RemoveWorld(const TObjectPtr<MWorld>& world)
	{
		RemoveWorld(world->GetGuid());
	}

	void Engine::RemoveWorld(const GUID& guid)
	{
		m_ActiveWorlds.erase(guid);
	}

	void Engine::RemoveInvalidObjectPointers()
	{
		m_InvalidMObjects.reserve(m_MObjects.size());
		for (auto& [guid, object] : m_MObjects)
		{
			if (object.IsExpired())
			{
				EngineLogger.Trace("Object with GUID {} has expired. It will get deleted shortly.", guid.ToString());
				m_InvalidMObjects.emplace_back(guid);
			}
		}
		for (GUID& guid : m_InvalidMObjects)
		{
			m_MObjects.erase(guid);
			m_TickingMObjects.erase(guid);
		}
		m_InvalidMObjects.clear();
	}

	void Engine::RegisterObject(const MObjectPtr& object)
	{
		ionassert(object);
		ionassert(!IsObjectRegistered(object->GetGuid()));

		const GUID& guid = object->GetGuid();

		ionassert(m_MObjects.find(guid) == m_MObjects.end(),
			"MObject of GUID {} has been registered already.", guid);
		ionassert(m_TickingMObjects.find(guid) == m_TickingMObjects.end());

		m_MObjects.emplace(guid, object);

		if (object->IsTickEnabled())
		{
			m_TickingMObjects.emplace(guid, object);
		}
	}

	bool Engine::IsObjectRegistered(const GUID& guid)
	{
		return m_MObjects.find(guid) != m_MObjects.end();
	}

	void Engine::SetObjectTickEnabled(const MObjectPtr& object, bool bEnabled)
	{
		ionassert(object);
		ionassert(IsObjectRegistered(object->GetGuid()));

		const GUID& guid = object->GetGuid();

		if (bEnabled)
		{
			m_TickingMObjects.emplace(guid, object);
		}
		else
		{
			m_TickingMObjects.erase(guid);
		}
	}

	Engine* g_Engine = new Engine;
}
