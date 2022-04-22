#pragma once

#include "Resource.h"

namespace Ion
{
	class ResourceRefCount;

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

		static void Register(const GUID& guid, const Asset& asset, const TShared<Resource>& resource);
		static void Unregister(Resource* resource);

		static TShared<Resource> Find(const GUID& guid);
		static TArray<TShared<Resource>> FindAssociatedResources(const Asset& asset);
		static const TArray<GUID>* FindAssociatedResourcesGUIDs(const Asset& asset);
		static bool IsAnyResourceAvailable(const Asset& asset);

		template<typename T>
		static TArray<TShared<T>> GetResourcesOfType();

		static const THashMap<GUID, TShared<Resource>>& GetAllRegisteredResources();

		/**
		 * @brief Checks if the Asset is in use by a Resource
		 * 
		 * @param asset The Asset to check for
		 */
		static bool IsRegistered(const GUID& guid);

		static inline constexpr size_t ResourceMapBucketCount = 64;

	private:
		ResourceManager();

	private:
		THashMap<GUID, TShared<Resource>> m_Resources;
		THashMap<Asset, TArray<GUID>> m_AssetToResources;
	};

	// ResourceManager inline implementation

	template<typename T>
	inline TArray<TShared<T>> ResourceManager::GetResourcesOfType()
	{
		ResourceManager& instance = Get();

		// @TODO: all the types should be indexed somewhere,
		// so that no searching is needed.

		TArray<TShared<T>> resources;
	
		for (auto& [guid, res] : instance.m_Resources)
		{
			if (dynamic_cast<T*>(res.get()))
			{
				resources.push_back(TStaticCast<T>(res));
			}
		}

		return resources;
	}
}
