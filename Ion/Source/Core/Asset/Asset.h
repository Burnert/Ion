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
		 * @brief Queries the AssetRegistry for the AssetDefinition object
		 * 
		 * @return If exists, a pointer to AssetDefinition associated
		 * with this handle, or else nullptr.
		 */
		AssetDefinition* FindAssetDefinition() const;

		/**
		 * @see FindAssetDefinition() 
		 */
		AssetDefinition* operator->();
		
		/**
		 * @see FindAssetDefinition() 
		 */
		AssetDefinition& operator*();

		/**
		 * @brief An Asset Handle initialized with an invalid GUID.
		 * Cannot be dereferenced.
		 */
		static const Asset InvalidHandle;

	private:
		explicit Asset(const GUID& guid);

	private:
		GUID m_Guid;

		friend class AssetRegistry;
		friend class AssetDefinition;
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
		 * specified in the constructor
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
		bool Parse(TUnique<XMLDocument>& xml, AssetInitializer& outInitializer) const;

	private:
		FilePath m_Path;
	};

	// Asset class inline implementation

	inline AssetDefinition* Asset::operator->()
	{
		return FindAssetDefinition();
	}

	inline AssetDefinition& Asset::operator*()
	{
		AssetDefinition* def = FindAssetDefinition();
		ionassertnd(def, "Cannot dereference an asset handle of an non-existing asset. {%s}", m_Guid);
		return *def;
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
