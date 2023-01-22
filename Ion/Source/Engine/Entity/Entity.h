#pragma once

#include "Matter/Object.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/SceneComponent.h"

namespace Ion
{
	REGISTER_LOGGER(MEntityLogger, "Engine::Entity");

	class MWorld;

	class ION_API MEntity : public MObject
	{
		MCLASS(MEntity)
		using Super = MObject;

	public:
		MEntity();

		TObjectPtr<MSceneComponent> GetRootComponent() const;

		void AddComponent(const TObjectPtr<MComponent>& component);
		MMETHOD(AddComponent, const TObjectPtr<MComponent>&)

		TArray<TObjectPtr<MComponent>> GetComponents() const;
		MMETHOD(GetComponents)

		bool IsSceneEntity() const;
		MMETHOD(IsSceneEntity)

	protected:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void Tick(float deltaTime) override;

		/**
		 * @brief Called right after the entity is spawned in a World.
		 * m_WorldContext will have been valid by the time this function gets called.
		 * 
		 * @see MWorld::SpawnEntity
		 */
		virtual void OnSpawn();

	protected:
		TObjectPtr<MSceneComponent> m_RootComponent;
		MFIELD(m_RootComponent)

	private:
		TWeakObjectPtr<MWorld> m_WorldContext;
		// @TODO: Can't do this because World.h cannot be included here
		//MFIELD(m_WorldContext)

		TArray<TObjectPtr<MComponent>> m_Components;
		MFIELD(m_Components)

		friend class MWorld;
	};

	FORCEINLINE TObjectPtr<MSceneComponent> MEntity::GetRootComponent() const
	{
		return m_RootComponent;
	}

	FORCEINLINE TArray<TObjectPtr<MComponent>> MEntity::GetComponents() const
	{
		return m_Components;
	}

	FORCEINLINE bool MEntity::IsSceneEntity() const
	{
		return m_RootComponent.IsValid();
	}
}
