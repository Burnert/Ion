#pragma once

#include "ResourceFwd.h"
#include "ResourceCommon.h"
#include "Asset/Asset.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetDefinition.h"

namespace Ion
{
	template<typename T>
	struct TIsResource : TIsBaseOf<Resource, T> { };

	template<typename T>
	static constexpr bool TIsResourceV = TIsResource<T>::value;

	// ------------------------------------------------------------
	// Base Resource
	// ------------------------------------------------------------

	template<typename T>
	using TFuncResourceOnTake = TFunction<void(const TSharedPtr<T>&)>;

	/**
	 * @brief Base Resource class
	 */
	class ION_API Resource : public TEnableSFT<Resource>
	{
	public:
		/**
		 * @return true if the resource render data is available.
		 */
		virtual bool IsLoaded() const = 0;

		/**
		 * @brief Get the Asset handle associated with the Resource
		 *
		 * @return Asset handle
		 */
		Asset GetAssetHandle() const;

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
		static TSharedPtr<T> Query(const Asset& asset);

	protected:
		Asset m_Asset;

		friend class ResourceManager;
	};

	inline Resource::Resource(const Asset& asset) :
		m_Asset(asset)
	{
	}

	template<typename T>
	inline TSharedPtr<T> Resource::Query(const Asset& asset)
	{
		static_assert(TIsBaseOfV<Resource, T>);

		using ResourceDescription = typename T::TResourceDescription;

		ionassert(asset);

		// Find a resource of type T
		TSharedPtr<T> resource = ResourceManager::FindAssociatedResource<T>(asset);
		if (resource)
		{
			ResourceLogger.Trace("Found a resource for asset \"{}\".", asset->GetVirtualPath());
			return resource;
		}

		ResourceDescription desc;
		GUID guid = GUID::Zero;

		resource = MakeSharedDC<T>([](T& res) {
			ResourceManager::Unregister(res);
		}, asset);

		ResourceManager::Register(resource);
		return resource;
	}

	inline Asset Resource::GetAssetHandle() const
	{
		return m_Asset;
	}
}
