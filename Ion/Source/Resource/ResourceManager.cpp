#include "IonPCH.h"

#include "ResourceManager.h"

namespace Ion
{
#if ION_DEBUG
	ResourceManager* g_Debug_ResourceManagerInstance;
#endif

	ResourceManager& ResourceManager::Get()
	{
		static ResourceManager* c_Instance = new ResourceManager;
#if ION_DEBUG
		g_Debug_ResourceManagerInstance = c_Instance;
#endif
		return *c_Instance;
	}

	void ResourceManager::Register(const GUID& guid, const Asset& asset, const ResourcePtr& resource)
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

		LOG_TRACE(L"Resource has been registered (asset \"{0}\").", asset->GetDefinitionPath().ToString());
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

		// @TODO: Fix - Gets called twice for some reason
		LOG_TRACE(L"Resource has been unregistered (asset \"{0}\").", assetHandle->GetDefinitionPath().ToString());
	}

	TArray<ResourcePtr> ResourceManager::FindAssociatedResources(const Asset& asset)
	{
		const TArray<GUID>* guids = FindAssociatedResourcesGUIDs(asset);
		if (guids)
		{
			// Get all the resources for the specified asset.
			TArray<ResourcePtr> resources;
			for (const GUID& guid : *guids)
			{
				ResourcePtr resource = Find(guid);
				if (resource)
				{
					resources.push_back(resource);
				}
			}
			return resources;
		}
		return TArray<ResourcePtr>();
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

	const THashMap<GUID, ResourceWeakPtr>& ResourceManager::GetAllRegisteredResources()
	{
		ResourceManager& instance = Get();

		return instance.m_Resources;
	}

	ResourcePtr ResourceManager::Find(const GUID& guid)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_Resources.find(guid);
		if (it != instance.m_Resources.end())
		{
			return it->second.Lock();
		}
		return ResourcePtr();
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
