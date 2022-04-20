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

		static void Register(const GUID& guid, const TShared<Resource>& resource);
		static void Unregister(Resource* resource);

		static TShared<Resource> Find(const Asset& asset);

		/**
		 * @brief Checks if the Asset is in use by a Resource
		 * 
		 * @param asset The Asset to check for
		 */
		static bool IsRegistered(const Asset& asset);

	private:
		THashMap<GUID, TShared<Resource>> m_Resources;
	};
}
