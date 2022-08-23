#include "IonPCH.h"

#include "Resource.h"
#include "ResourceManager.h"
#include "Core/Asset/AssetRegistry.h"

namespace Ion
{
	namespace _Detail
	{
		bool ResourceRefHelper::IsRegistered(Resource* resource) noexcept
		{
			return ResourceManager::IsRegistered(resource);
		}

		size_t ResourceRefHelper::IncRef(Resource* resource) noexcept
		{
			ResourceControlBlock* block = ResourceManager::GetControlBlock(resource);
			return ++block->RefCount;
		}

		size_t ResourceRefHelper::DecRef(Resource* resource) noexcept
		{
			ResourceControlBlock* block = ResourceManager::GetControlBlock(resource);
			size_t count = --block->RefCount;
			if (count == 0)
			{
				ResourceManager::Unregister(resource);
			}
			return count;
		}
	}
}
