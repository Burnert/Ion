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

		static void Register(const GUID& guid, const Asset& asset, const ResourcePtr& resource);
		static void Unregister(Resource* resource);

		static ResourcePtr Find(const GUID& guid);
		static TArray<ResourcePtr> FindAssociatedResources(const Asset& asset);
		static const TArray<GUID>* FindAssociatedResourcesGUIDs(const Asset& asset);
		static bool IsAnyResourceAvailable(const Asset& asset);

		template<typename T>
		static TArray<TResourcePtr<T>> GetResourcesOfType();

		static const THashMap<GUID, ResourceWeakPtr>& GetAllRegisteredResources();

		/**
		 * @brief Checks if the Asset is in use by a Resource
		 * 
		 * @param asset The Asset to check for
		 */
		static bool IsRegistered(const GUID& guid);

		static inline constexpr size_t ResourceMapBucketCount = 64;

	private:
		ResourceManager();

		static ResourcePtr Find();

	private:
		THashMap<GUID, ResourceWeakPtr> m_Resources;
		THashMap<Asset, TArray<GUID>> m_AssetToResources;

		friend class ResourceMemory;
	};

	// ResourceManager inline implementation

	template<typename T>
	inline TArray<TResourcePtr<T>> ResourceManager::GetResourcesOfType()
	{
		ResourceManager& instance = Get();

		// @TODO: all the types should be indexed somewhere,
		// so that no searching is needed.

		TArray<TResourcePtr<T>> resources;
	
		for (auto& [guid, res] : instance.m_Resources)
		{
			ResourcePtr ref = res.Lock();
			if (dynamic_cast<T*>(ref.GetRaw()))
			{
				resources.push_back(TStaticResourcePtrCast<T>(ref));
			}
		}

		return resources;
	}
}
