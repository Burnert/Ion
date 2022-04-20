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
		Get().m_Resources.emplace(guid, resource);
	}

	void ResourceManager::Unregister(Resource* resource)
	{
		// @TODO: Yeah sure
		Get().m_Resources.erase(GUID());
	}

	TShared<Resource> ResourceManager::Find(const Asset& asset)
	{
		const GUID& guid = asset.GetGuid();

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
