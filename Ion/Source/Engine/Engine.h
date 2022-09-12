#pragma once

#include "EngineCore.h"
#include "World.h"
#include "Entity/Entity.h"

namespace Ion
{
	REGISTER_LOGGER(EngineLogger, "Engine");

	class ION_API Engine
	{
	public:
		void Init();
		void Shutdown();
		void Update(float deltaTime);

		// Create a world using an initializer
		World* CreateWorld(const WorldInitializer& initializer);
		// Create a world from an archive
		World* CreateWorld(Archive& ar);

		World* CreateWorldFromMapAsset(const Asset& mapAsset);

		void DestroyWorld(World* world);
		World* FindWorld(const GUID& worldGuid) const;

		void BuildRendererData(float deltaTime);

		float GetGlobalDeltaTime() const;

	private:
		TArray<World*> m_RegisteredWorlds;

		float m_DeltaTime;
	};

	/* Global Engine pointer. Assume it is never null. */
	extern ION_API Engine* g_Engine;

	// Inline definitions

	inline float Engine::GetGlobalDeltaTime() const
	{
		return m_DeltaTime;
	}
}
