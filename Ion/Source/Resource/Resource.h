#pragma once

#include "ResourceFwd.h"
#include "ResourceCommon.h"
#include "Core/Asset/Asset.h"
#include "Core/Asset/AssetRegistry.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Base Resource
	// ------------------------------------------------------------

	template<typename T>
	using TFuncResourceOnTake = TFunction<void(const T&)>;

	/**
	 * @brief Base Resource class
	 */
	class ION_API Resource
	{
	public:
		~Resource();

		virtual bool IsLoaded() const = 0;

		/**
		 * @brief Get the Resource Guid
		 *
		 * @return Resource Guid
		 */
		const GUID& GetGuid() const;

		/**
		 * @brief Get the Asset handle associated with the Resource
		 *
		 * @return Asset handle
		 */
		Asset GetAssetHandle() const;

	protected:
		Resource(const GUID& guid, const Asset& asset);

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
		GUID m_Guid;
		Asset m_Asset;

		friend class ResourceManager;
	};

	template<typename T>
	inline TShared<T> Resource::Query(const Asset& asset)
	{
		static_assert(TIsBaseOfV<Resource, T>);

		TShared<Resource> resource;

		// Find a resource of type T
		if (ResourceManager::IsAnyResourceAvailable(asset))
		{
			TArray<TShared<Resource>> resources = ResourceManager::FindAssociatedResources(asset);
			for (TShared<Resource>& res : resources)
			{
				// Check the actual type of the resource
				if (dynamic_cast<T*>(res.get()))
				{
					resource = res;
				}
			}
		}

		if (resource)
		{
			return TStaticCast<T>(resource);
		}
		else
		{
			typename T::TResourceDescription desc;
			GUID guid = GUID::Zero;
			if (T::ParseAssetFile(asset->GetDefinitionPath(), guid, desc))
			{
				// Register the new resource, if it doesn't exist.
				TShared<T> newResource = MakeShareable(new T(guid, asset, desc));
				// Register the resource using the asset's Guid.
				ResourceManager::Register(guid, asset, newResource);
				return newResource;
			}
			return nullptr;
		}
	}

	inline const GUID& Resource::GetGuid() const
	{
		return m_Guid;
	}

	inline Asset Resource::GetAssetHandle() const
	{
		return m_Asset;
	}
}
