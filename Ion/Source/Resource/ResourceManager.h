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
}
