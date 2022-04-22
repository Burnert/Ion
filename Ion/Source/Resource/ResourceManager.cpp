#include "IonPCH.h"

#include "ResourceManager.h"

namespace Ion
{
	ResourceManager& ResourceManager::Get()
	{
		static ResourceManager* c_Instance = new ResourceManager;
		return *c_Instance;
	}

	void ResourceManager::Register(const GUID& guid, const Asset& asset, const TShared<Resource>& resource)
	{
		ResourceManager& instance = Get();

		if (Find(guid))
		{
			LOG_ERROR("Cannot register a resource.\n\nA resource with guid {{{0}}} already exists.", guid.ToString());
			return;
		}
		instance.m_Resources.emplace(guid, resource);
		// Associate the resource with the asset
		instance.m_AssetToResources[asset].push_back(guid);
	}

	void ResourceManager::Unregister(Resource* resource)
	{
		ionassert(resource);

		ResourceManager& instance = Get();

		const GUID& resourceGuid = resource->GetGuid();
		const Asset& assetHandle = resource->GetAssetHandle();

		// Erase the resource from the associated asset
		TArray<GUID>& resources = instance.m_AssetToResources[assetHandle];
		auto it = std::find(resources.begin(), resources.end(), resourceGuid);
		if (it != resources.end())
		{
			resources.erase(it);
			// Delete the asset from the map if there
			// are no more resources associated with it.
			if (resources.empty())
			{
				instance.m_AssetToResources.erase(assetHandle);
			}
		}

		instance.m_Resources.erase(resourceGuid);
	}

	TArray<TShared<Resource>> ResourceManager::FindAssociatedResources(const Asset& asset)
	{
		const TArray<GUID>* guids = FindAssociatedResourcesGUIDs(asset);
		if (guids)
		{
			// Get all the resources for the specified asset.
			TArray<TShared<Resource>> resources;
			for (const GUID& guid : *guids)
			{
				TShared<Resource> resource = Find(guid);
				if (resource)
				{
					resources.push_back(resource);
				}
			}
			return resources;
		}
		return TArray<TShared<Resource>>();
	}

	const TArray<GUID>* ResourceManager::FindAssociatedResourcesGUIDs(const Asset& asset)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_AssetToResources.find(asset);
		if (it != instance.m_AssetToResources.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	bool ResourceManager::IsAnyResourceAvailable(const Asset& asset)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_AssetToResources.find(asset);
		return it != instance.m_AssetToResources.end();
	}

	const THashMap<GUID, TShared<Resource>>& ResourceManager::GetAllRegisteredResources()
	{
		ResourceManager& instance = Get();

		return instance.m_Resources;
	}

	TShared<Resource> ResourceManager::Find(const GUID& guid)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_Resources.find(guid);
		if (it != instance.m_Resources.end())
		{
			return it->second;
		}
		return TShared<Resource>();
	}

	bool ResourceManager::IsRegistered(const GUID& guid)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_Resources.find(guid);
		return it != instance.m_Resources.end();
	}

	ResourceManager::ResourceManager() :
		m_Resources(ResourceMapBucketCount),
		m_AssetToResources(ResourceMapBucketCount)
	{
	}
}
