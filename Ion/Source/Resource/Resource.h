#pragma once

#include "ResourceFwd.h"
#include "ResourceCommon.h"
#include "Asset/Asset.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetDefinition.h"

namespace Ion
{
	class Resource;

	template<typename T>
	struct TIsResource : TIsBaseOf<Resource, T> { };

	template<typename T>
	static constexpr bool TIsResourceV = TIsResource<T>::value;

	template<typename T>
	class TResourceRef;

	namespace _Detail
	{
		class ResourceRefHelper
		{
			static bool IsRegistered(Resource* resource) noexcept;

			static size_t IncRef(Resource* resource) noexcept;
			static size_t DecRef(Resource* resource) noexcept;

			template<typename T>
			friend class TResourceRef;
		};
	}

#define DEFINE_RESOURCE_AS_REF(type) inline TResourceRef<type> AsRef() { return TResourceRef<type>(this); }

	template<typename T>
	class TResourceRef
	{
	public:
		static_assert(TIsResourceV<T>);

		TResourceRef() noexcept;
		TResourceRef(T* resource) noexcept;

		TResourceRef(const TResourceRef& other) noexcept;
		TResourceRef(TResourceRef&& other) noexcept;

		~TResourceRef() noexcept;

		TResourceRef& operator=(const TResourceRef& other) noexcept;
		TResourceRef& operator=(TResourceRef&& other) noexcept;

		bool operator==(const TResourceRef& other) const noexcept;

		void Swap(TResourceRef& other) noexcept;

		NODISCARD T* operator->() const noexcept;
		NODISCARD T& operator*() const noexcept;

		operator bool() const noexcept;

	private:
		T* m_ResourcePtr;
	};

#define _UNREGISTERED_RESOURCE_REF_CREATION_MESSAGE "Tried to create a TResourceRef of an unregistered resource."

	template<typename T>
	inline TResourceRef<T>::TResourceRef() noexcept :
		m_ResourcePtr(nullptr)
	{
	}

	template<typename T>
	inline TResourceRef<T>::TResourceRef(T* resource) noexcept :
		m_ResourcePtr(resource)
	{
		ionverify(_Detail::ResourceRefHelper::IsRegistered(resource), _UNREGISTERED_RESOURCE_REF_CREATION_MESSAGE);
		_Detail::ResourceRefHelper::IncRef(resource);
	}

	template<typename T>
	inline TResourceRef<T>::TResourceRef(const TResourceRef& other) noexcept
	{
		if (other.m_ResourcePtr)
		{
			ionassert(_Detail::ResourceRefHelper::IsRegistered(other.m_ResourcePtr), _UNREGISTERED_RESOURCE_REF_CREATION_MESSAGE);
			_Detail::ResourceRefHelper::IncRef(other.m_ResourcePtr);
		}
		m_ResourcePtr = other.m_ResourcePtr;
	}

	template<typename T>
	inline TResourceRef<T>::TResourceRef(TResourceRef&& other) noexcept
	{
		ionassert(!other.m_ResourcePtr || _Detail::ResourceRefHelper::IsRegistered(other.m_ResourcePtr), _UNREGISTERED_RESOURCE_REF_CREATION_MESSAGE);
		m_ResourcePtr = other.m_ResourcePtr;
		other.m_ResourcePtr = nullptr;
	}

	template<typename T>
	inline TResourceRef<T>::~TResourceRef() noexcept
	{
		if (m_ResourcePtr)
		{
			ionassert(_Detail::ResourceRefHelper::IsRegistered(m_ResourcePtr));
			_Detail::ResourceRefHelper::DecRef(m_ResourcePtr);
		}
	}

	template<typename T>
	inline TResourceRef<T>& TResourceRef<T>::operator=(const TResourceRef& other) noexcept
	{
		TResourceRef(other).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TResourceRef<T>&  TResourceRef<T>::operator=(TResourceRef&& other) noexcept
	{
		TResourceRef(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	inline bool TResourceRef<T>::operator==(const TResourceRef& other) const noexcept
	{
		return m_ResourcePtr == other.m_ResourcePtr;
	}

	template<typename T>
	inline void TResourceRef<T>::Swap(TResourceRef& other) noexcept
	{
		std::swap(m_ResourcePtr, other.m_ResourcePtr);
	}

	template<typename T>
	inline T* TResourceRef<T>::operator->() const noexcept
	{
		ionassert(m_ResourcePtr);
		ionassert(_Detail::ResourceRefHelper::IsRegistered(m_ResourcePtr));
		return m_ResourcePtr;
	}

	template<typename T>
	inline T& TResourceRef<T>::operator*() const noexcept
	{
		ionassert(m_ResourcePtr);
		ionassert(_Detail::ResourceRefHelper::IsRegistered(m_ResourcePtr));
		return *m_ResourcePtr;
	}

	template<typename T>
	inline TResourceRef<T>::operator bool() const noexcept
	{
		return m_ResourcePtr;
	}

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
		static TResourceRef<T> Query(const Asset& asset);

	protected:
		Asset m_Asset;

		friend class ResourceManager;
	};

	inline Resource::Resource(const Asset& asset) :
		m_Asset(asset)
	{
	}

	template<typename T>
	inline TResourceRef<T> Resource::Query(const Asset& asset)
	{
		static_assert(TIsBaseOfV<Resource, T>);

		using ResourceDescription = typename T::TResourceDescription;

		ionassert(asset);

		// Find a resource of type T
		TResourceRef<T> resource = ResourceManager::FindAssociatedResource<T>(asset);

		if (resource)
		{
			ResourceLogger.Trace("Found a resource for asset \"{}\".", asset->GetVirtualPath());
			return resource;
		}

		ResourceDescription desc;
		GUID guid = GUID::Zero;
		if (T::ParseAssetFile(asset, guid, desc))
		{
			return ResourceManager::Register(new T(asset, desc));
		}

		return TResourceRef<T>();
	}

	inline Asset Resource::GetAssetHandle() const
	{
		return m_Asset;
	}
}
