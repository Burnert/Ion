#include "IonPCH.h"

#include "NullEntity.h"

namespace Ion
{
	MNullEntity::MNullEntity()
	{
		m_RootComponent = MObject::ConstructDefault<MSceneComponent>();
	}
}
