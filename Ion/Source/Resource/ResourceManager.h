#pragma once

#include "Resource.h"

namespace Ion
{
	/**
	 * @brief Resource Manager class
	 * 
	 */
	class ION_API ResourceManager
	{
	public:
		/**
		 * @brief Get the Resource Manager singleton
		 * 
		 * @return Resource Manager object
		 */
		static ResourceManager& Get();

		template<typename T, TEnableIfT<TIsResourceV<T>>* = 0>
		static void Register(const TSharedPtr<T>& resource);
		static void Unregister(Resource& resource);

		template<typename T, TEnableIfT<TIsResourceV<T>>* = 0>
		static TSharedPtr<T> FindAssociatedResource(const Asset& asset);
		static bool IsAnyResourceAvailable(const Asset& asset);

		template<typename T, TEnableIfT<TIsResourceV<T>>* = 0>
		static TArray<TSharedPtr<T>> GetResourcesOfType();

		/**
		 * @brief Checks if the Asset is in use by a Resource
		 * 
		 * @param asset The Asset to check for
		 */
		static bool IsRegistered(const Resource& resource);

		static inline constexpr size_t ResourceMapBucketCount = 256;

	private:
		ResourceManager();

	private:
		THashMap<Resource*, TWeakPtr<Resource>> m_Resources;
		THashMap<Asset, TArray<TWeakPtr<Resource>>> m_AssetToResources;

		static ResourceManager* s_Instance;

		friend class ResourceMemory;
		template<typename T>
		friend class TResoureRef;
	};

	// ResourceManager inline implementation

	template<typename T, TEnableIfT<TIsResourceV<T>>*>
	inline void ResourceManager::Register(const TSharedPtr<T>& resource)
	{
		ionassert(!IsRegistered(*resource));

		ResourceManager& instance = Get();

		instance.m_Resources.emplace(resource.Raw(), TWeakPtr<Resource>(resource));

		const Asset& asset = resource->GetAssetHandle();
		// Associate the resource with the asset
		instance.m_AssetToResources[asset].push_back(resource);

		ResourceLogger.Info("Registered resource \"{}\".", asset->GetVirtualPath());
	}

	template<typename T, TEnableIfT<TIsResourceV<T>>*>
	TSharedPtr<T> ResourceManager::FindAssociatedResource(const Asset& asset)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_AssetToResources.find(asset);
		if (it == instance.m_AssetToResources.end())
			return nullptr;

		const TArray<TWeakPtr<Resource>>& resources = it->second;
		ionassert(!resources.empty());

		for (const TWeakPtr<Resource>& resource : resources)
		{
			ionassert(!resource.IsExpired());
			
			if (dynamic_cast<T*>(resource.Raw()))
			{
				return PtrCast<T>(resource.Lock());
			}
		}
		return nullptr;
	}

	template<typename T, TEnableIfT<TIsResourceV<T>>*>
	inline TArray<TSharedPtr<T>> ResourceManager::GetResourcesOfType()
	{
		ResourceManager& instance = Get();

		// @TODO: all the types should be indexed somewhere,
		// so that no searching is needed.

		TArray<TSharedPtr<T>> resources;
	
		for (auto& [raw, weak] : instance.m_Resources)
		{
			ionassert(!weak.IsExpired());

			if (dynamic_cast<T*>(weak.Raw()))
			{
				resources.emplace_back(PtrCast<T>(weak.Lock()));
			}
		}

		return resources;
	}
}
