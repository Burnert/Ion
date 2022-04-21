#pragma once

#include "Core/Asset/Asset.h"
#include "Core/Asset/AssetRegistry.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Base Resource
	// ------------------------------------------------------------

	template<typename T>
	using TFuncResourceOnTake = TFunction<void(TShared<T>)>;

	/**
	 * @brief Base Resource class
	 */
	class ION_API Resource
	{
	public:
		~Resource();

	protected:
		Resource(const Asset& asset);

		/**
		 * @brief Query the Resource Manager for a Resource
		 * of type T associated with the specified asset.
		 * 
		 * @tparam T Resource subtype
		 * @param asset Asset associated with the Resource
		 * @return Shared pointer to the Resource
		 */
		template<typename T>
		static TShared<T> Query(const Asset& asset);

	protected:
		Asset m_Asset;

		friend class ResourceManager;
	};

	template<typename T>
	inline TShared<T> Resource::Query(const Asset& asset)
	{
		static_assert(TIsBaseOfV<Resource, T>);

		if (TShared<Resource> resource = ResourceManager::Find(asset))
		{
			return TStaticCast<T>(resource);
		}
		else
		{
			// Register the new resource, if it doesn't exist.
			TShared<T> newResource = MakeShareable(new T(asset));
			ResourceManager::Register(asset.GetGuid(), newResource);
			return newResource;
		}
	}
}
