#include "IonPCH.h"

#include "ResourceMemory.h"
#include "ResourceManager.h"

namespace Ion
{
	void ResourceHelper::UnregisterResource(Resource* ptr)
	{
		ResourceManager::Unregister(ptr);
	}

	uint32 ResourceMemory::IncRef(const Resource& resource)
	{
		const GUID& guid = resource.GetGuid();
		TResourcePtrBase<Resource> resourcePtr = ResourceManager::Find(guid);

		if (!resourcePtr)
			return (uint32)-1;

		return IncRef(resourcePtr);
	}

	uint32 ResourceMemory::DecRef(const Resource& resource)
	{
		const GUID& guid = resource.GetGuid();
		TResourcePtrBase<Resource> resourcePtr = ResourceManager::Find(guid);

		if (!resourcePtr)
			return (uint32)-1;

		return DecRef(resourcePtr);
	}
}
