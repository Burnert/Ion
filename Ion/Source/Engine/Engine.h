#pragma once

#include "EngineCore.h"
#include "Matter/Object.h"
#include "World.h"
#include "Entity/EntityOld.h"

namespace Ion
{
	REGISTER_LOGGER(EngineLogger, "Engine");

	class EngineMObjectInterface
	{
		static void RegisterObject(const MObjectPtr& object);
		static void SetObjectTickEnabled(const MObjectPtr& object, bool bEnabled);

		friend class MObject;
	};

	class ION_API Engine
	{
	public:
		Engine();

		void Init();
		void Shutdown();

		/** Main Engine loop */
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
		void RemoveInvalidObjectPointers();

		// EngineMObjectInterface:

		void RegisterObject(const MObjectPtr& object);
		bool IsObjectRegistered(const GUID& guid);
		void SetObjectTickEnabled(const MObjectPtr& object, bool bEnabled);

		// End of EngineMObjectInterface

	private:
		TArray<World*> m_RegisteredWorlds;

		// @TODO: Use contiguous collections for faster iteration
		THashMap<GUID, MWeakObjectPtr> m_MObjects;
		THashMap<GUID, MWeakObjectPtr> m_TickingMObjects;
		mutable TArray<GUID> m_InvalidMObjects;

		float m_DeltaTime;

		friend class EngineMObjectInterface;
	};

	/* Global Engine pointer. Assume it is never null. */
	extern ION_API Engine* g_Engine;

	// Inline definitions

	FORCEINLINE float Engine::GetGlobalDeltaTime() const
	{
		return m_DeltaTime;
	}
}
