#include "IonPCH.h"

#include "ResourceMemory.h"
#include "ResourceManager.h"

namespace Ion
{
	void ResourceHelper::UnregisterResource(Resource* ptr)
	{
		ResourceManager::Unregister(ptr);
	}
}
