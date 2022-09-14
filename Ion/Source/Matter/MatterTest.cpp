#include "IonPCH.h"

#include "Object.h"
#include "Engine/Entity/Entity.h"

namespace Ion::Test
{
	void MatterTest()
	{
		Entity* entity = MObject::New<Entity>();
		ionassert(entity);
		ionassert(Entity::MatterRF_IntField->GetClass()->Is<Entity>());
		ionassert(Entity::MatterRF_IntField->GetType() == MatterRT_int32);

		MClass* entityClass = entity->GetClass();
		ionassert(entityClass);
		ionassert(entityClass->GetName() == "C_Entity");
		ionassert(entityClass->Is<Entity>());
		ionassert(entityClass->Is(Entity::StaticClass()));
	}
}
