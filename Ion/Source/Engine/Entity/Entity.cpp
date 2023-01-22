#include "IonPCH.h"

#include "Entity.h"
#include "Engine/World.h"

namespace Ion
{
	MEntity::MEntity()
	{
	}

	void MEntity::AddComponent(const TObjectPtr<MComponent>& component)
	{
		if (std::find(m_Components.begin(), m_Components.end(), component) != m_Components.end())
		{
			MEntityLogger.Warn("Component {} is already owned by {}.", component->GetName(), GetName());
			return;
		}
		m_Components.emplace_back(component);
	}

	void MEntity::OnCreate()
	{
	}
	
	void MEntity::OnDestroy()
	{
	}
	
	void MEntity::Tick(float deltaTime)
	{
	}

	void MEntity::OnSpawn()
	{
		ionassert(m_WorldContext);

		MEntityLogger.Trace("Entity {} has been spawned in world {}.", GetName(), m_WorldContext.Raw()->GetName());
	}
}
