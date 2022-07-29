#pragma once

#include "AssetCommon.h"

namespace Ion
{
	class ION_API Asset
	{
	public:
		/**
		 * @brief Creates an empty (null) asset handle (not invalid!)
		 * 
		 * @details Use AssetFinder to create an asset handle or query
		 * the AssetRegistry to get a handle to an existing asset.
		 */
		Asset();
		~Asset();

		/**
		 * @brief Find the existing asset by GUID.
		 * 
		 * @param guid GUID of the asset
		 * @return Asset handle, invalid handle if the asset with that GUID does not exist.
		 */
		//static Asset Find(const GUID& guid);

		/**
		 * @brief Find the existing asset by virtual path.
		 *
		 * @param virtualPath a VP to an asset (e.g. "<Engine>/Materials/DefaultMaterial")
		 * @return Asset handle, invalid handle if the asset with that path does not exist.
		 */
		static Asset Resolve(const String& virtualPath);

		static FilePath ResolveVirtualPath(const String& virtualPath);

		/**
		 * @brief Queries the AssetRegistry for the AssetDefinition object
		 * 
		 * @return If exists, a pointer to AssetDefinition associated
		 * with this handle, or else nullptr.
		 */
		AssetDefinition* GetAssetDefinition() const;

		/**
		 * @see GetAssetDefinition()
		 */
		AssetDefinition* operator->() const;
		
		/**
		 * @see GetAssetDefinition()
		 */
		AssetDefinition& operator*() const;

		/**
		 * @brief Is the Asset handle valid (can be null)
		 */
		bool IsValid() const;

		bool IsNone() const;

		/**
		 * @brief Checks if the Asset handle is valid and not null
		 */
		operator bool() const;

		/**
		 * @brief Checks if the asset handles are the same.
		 * (if paths are the same)
		 * 
		 * @param other Other asset
		 */
		bool operator==(const Asset& other) const;
		bool operator!=(const Asset& other) const;

		/**
		 * @brief An Asset Handle initialized with a nullptr, that cannot serve as a None handle.
		 * Cannot be dereferenced.
		 */
		static const Asset InvalidHandle;
		static const Asset None;

	private:
		explicit Asset(AssetDefinition* asset);
		struct InvalidInitializerT { };
		Asset(InvalidInitializerT);

		static bool Parse(AssetInitializer& inOutInitializer);

	private:
		/**
		 * If the 0-th flag is set, it means the handle is <None>.
		 */
		TMetaPointer<AssetDefinition> m_AssetPtr;

		friend class AssetRegistry;
		friend class AssetDefinition;

	public:
		struct Hasher
		{
			size_t operator()(const Asset& asset) const noexcept
			{
				return THash<void*>()(asset.m_AssetPtr.Get());
			}
		};
	};

	// Asset class inline implementation

	inline AssetDefinition* Asset::operator->() const
	{
		AssetDefinition* def = GetAssetDefinition();
		ionassertnd(def, "Cannot dereference an asset handle of a non-existing asset. {%p}", m_AssetPtr.Get());
		return def;
	}

	inline AssetDefinition& Asset::operator*() const
	{
		AssetDefinition* def = GetAssetDefinition();
		ionassertnd(def, "Cannot dereference an asset handle of a non-existing asset. {%p}", m_AssetPtr.Get());
		return *def;
	}

	inline bool Asset::IsValid() const
	{
		return m_AssetPtr || IsNone();
	}

	inline bool Asset::IsNone() const
	{
		return m_AssetPtr.GetMetaFlag<0>();
	}

	inline Asset::operator bool() const
	{
		return m_AssetPtr && !IsNone();
	}

	inline bool Asset::operator==(const Asset& other) const
	{
		return (m_AssetPtr == other.m_AssetPtr) || (IsNone() && other.IsNone());
	}

	inline bool Asset::operator!=(const Asset& other) const
	{
		return !operator==(other);
	}
}

/**
 * @brief std::hash specialization for Asset handle type
 */
template<>
struct std::hash<Ion::Asset>
{
	size_t operator()(const Ion::Asset& asset) const noexcept {

		return Ion::Asset::Hasher()(asset);
	}
};
