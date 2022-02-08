#pragma once

#include "Core/Core.h"
#include "World.h"
#include "Entity.h"

namespace Ion
{
	class ION_API Engine
	{
	public:
		void Init();
		void Shutdown();
		void Update(float deltaTime);

		World* CreateWorld(const WorldInitializer& initializer);
		void DestroyWorld(World* world);

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