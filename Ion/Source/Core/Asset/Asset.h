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
		static Asset Find(const GUID& guid);

		/**
		 * @brief Queries the AssetRegistry for the AssetDefinition object
		 * 
		 * @return If exists, a pointer to AssetDefinition associated
		 * with this handle, or else nullptr.
		 */
		AssetDefinition* FindAssetDefinition() const;

		const GUID& GetGuid() const;

		/**
		 * @see FindAssetDefinition()
		 */
		AssetDefinition* operator->() const;
		
		/**
		 * @see FindAssetDefinition()
		 */
		AssetDefinition& operator*() const;

		/**
		 * @brief Is the Asset handle valid (can be null)
		 */
		bool IsValid() const;

		/**
		 * @brief Checks if the Asset handle is valid and not null
		 */
		operator bool() const;

		/**
		 * @brief Checks if the asset handles are the same.
		 * (if GUIDs are the same)
		 * 
		 * @param other Other asset
		 */
		bool operator==(const Asset& other) const;
		bool operator!=(const Asset& other) const;

		/**
		 * @brief An Asset Handle initialized with an invalid GUID.
		 * Cannot be dereferenced.
		 */
		static const Asset InvalidHandle;

	private:
		explicit Asset(const GUID& guid);

	private:
		GUID m_Guid;
#if ION_DEBUG
		AssetDefinition* m_DebugDefinition;
#endif

		friend class AssetRegistry;
		friend class AssetDefinition;

	public:
		struct Hasher
		{
			size_t operator()(const Asset& asset) const noexcept
			{
				return THash<GUID>()(asset.GetGuid());
			}
		};
	};

	/**
	 * @brief Utility class to create asset handles
	 */
	class ION_API AssetFinder
	{
	public:
		/**
		 * @brief Construct a new Asset Finder object
		 * 
		 * @param path Path to the .iasset file
		 */
		explicit AssetFinder(const FilePath& path);

		/**
		 * @brief Tries to parse the .iasset (asset definition) file
		 * specified in the constructor.
		 * 
		 * @details If the asset is found and successfully parsed,
		 * it is added to the Asset Registry.
		 * 
		 * @return Asset handle - if the path is invalid or some other
		 * error occurs, the returned handle will be invalid.
		 */
		Asset Resolve() const;

		/**
		 * @brief Checks if the file provided in the constructor exists.
		 */
		bool Exists() const;

		/** @brief Same as Exists */
		operator bool() const;

	private:
		bool Parse(TShared<XMLDocument>& xml, AssetInitializer& outInitializer) const;

	private:
		FilePath m_Path;
	};

	// Asset class inline implementation

	inline const GUID& Asset::GetGuid() const
	{
		return m_Guid;
	}

	inline AssetDefinition* Asset::operator->() const
	{
		return FindAssetDefinition();
	}

	inline AssetDefinition& Asset::operator*() const
	{
		AssetDefinition* def = FindAssetDefinition();
		ionassertnd(def, "Cannot dereference an asset handle of an non-existing asset. {%s}", m_Guid);
		return *def;
	}

	inline bool Asset::IsValid() const
	{
		return !m_Guid.IsInvalid();
	}

	inline Asset::operator bool() const
	{
		return IsValid() && !m_Guid.IsZero();
	}

	inline bool Asset::operator==(const Asset& other) const
	{
		return m_Guid == other.m_Guid;
	}

	inline bool Asset::operator!=(const Asset& other) const
	{
		return m_Guid != other.m_Guid;
	}

	// AssetFinder class inline implementation

	inline bool AssetFinder::Exists() const
	{
		return m_Path.IsFile();
	}

	inline AssetFinder::operator bool() const
	{
		return Exists();
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
