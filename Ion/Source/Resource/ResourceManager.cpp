#include "IonPCH.h"

#include "ResourceManager.h"

namespace Ion
{
	ResourceManager& ResourceManager::Get()
	{
		static ResourceManager* c_Instance = new ResourceManager;
		return *c_Instance;
	}

	void ResourceManager::Register(const GUID& guid, const TShared<Resource>& resource)
	{
		if (Find(guid))
		{
			LOG_ERROR("Cannot register a resource.\n\nA resource with guid {{{0}}} already exists.", guid.ToString());
			return;
		}
		Get().m_Resources.emplace(guid, resource);
	}

	void ResourceManager::Unregister(Resource* resource)
	{
		Get().m_Resources.erase(resource->m_Asset.GetGuid());
	}

	TShared<Resource> ResourceManager::Find(const Asset& asset)
	{
		return Find(asset.GetGuid());
	}

	TShared<Resource> ResourceManager::Find(const GUID& guid)
	{
		auto it = Get().m_Resources.find(guid);
		if (it != Get().m_Resources.end())
		{
			return it->second;
		}
		return TShared<Resource>();
	}

	bool ResourceManager::IsRegistered(const Asset& asset)
	{
		const GUID& guid = asset.GetGuid();
		
		auto it = Get().m_Resources.find(guid);
		return it != Get().m_Resources.end();
	}
}
