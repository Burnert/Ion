#include "IonPCH.h"

#include "ResourceManager.h"

namespace Ion
{
	ResourceManager* ResourceManager::s_Instance;

	ResourceManager& ResourceManager::Get()
	{
		if (!s_Instance)
			return *(s_Instance = new ResourceManager);
		return *s_Instance;
	}

	void ResourceManager::Unregister(Resource* resource)
	{
		ionassert(resource);
		ionassert(IsRegistered(resource));

		ResourceManager& instance = Get();

		const Asset& assetHandle = resource->GetAssetHandle();

		// Erase the resource from the associated asset
		TArray<Resource*>& resources = instance.m_AssetToResources[assetHandle];
		auto it = std::find(resources.begin(), resources.end(), resource);
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

		instance.m_Resources.erase(resource);

		ResourceLogger.Info("Unregistered resource \"{}\".", assetHandle->GetVirtualPath());
	}

	bool ResourceManager::IsAnyResourceAvailable(const Asset& asset)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_AssetToResources.find(asset);
		return it != instance.m_AssetToResources.end();
	}

	bool ResourceManager::IsRegistered(Resource* resource)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_Resources.find(resource);
		return it != instance.m_Resources.end();
	}

	ResourceManager::ResourceManager() :
		m_Resources(ResourceMapBucketCount),
		m_AssetToResources(ResourceMapBucketCount)
	{
	}
}
