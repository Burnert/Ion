#pragma once

#include "AssetCommon.h"

namespace Ion
{
	class ION_API Asset
	{
	public:
		struct VirtualRoot
		{
			static constexpr const char* Engine  = "[Engine]";
			static constexpr const char* Shaders = "[Shaders]";
			static constexpr const char* Game    = "[Game]";
		};

		static constexpr const char FileExtension[] = ".iasset";
		static constexpr const char FileExtensionNoDot[] = "iasset";

		static const Asset None;

		/**
		 * @brief Creates an empty (None) asset handle (not invalid!)
		 * You can also use Asset::None for that purpose.
		 */
		Asset();
		~Asset();

		/**
		 * @brief Retrieve an asset handle for the specified virtual path.
		 * 
		 * @details If the asset has not been registered yet, it will try to find the asset file on disk.
		 * If the asset's already registered, it will just return the handle to that asset.
		 *
		 * @param virtualPath a virtual path to an asset (e.g. "[Engine]/Materials/DefaultMaterial")
		 * @return Asset handle, FileNotFoundError if the asset with that path does not exist, or IOError if the asset cannot be parsed.
		 */
		static Result<Asset, IOError, FileNotFoundError> Resolve(const String& virtualPath);

		static Result<Asset, IOError, FileNotFoundError> RegisterExternal(const FilePath& path, const String& customVirtualPath);

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
		 * @brief Is the Asset handle not None
		 */
		bool IsValid() const;

		/**
		 * @brief Checks if the Asset handle is not None
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

		static bool IsVirtualRoot(const StringView& root);
		static bool IsStandardVirtualRoot(const StringView& root);
		static bool IsValidVirtualPath(const String& virtualPath);
		static String GetRootOfVirtualPath(const String& virtualPath);
		static String GetRestOfVirtualPath(const String& virtualPath);

	private:
		explicit Asset(AssetDefinition* asset);

		static bool Parse(AssetInitializer& inOutInitializer);

		static Result<Asset, IOError, FileNotFoundError> RegisterAsset(const FilePath& path, const String& virtualPath);

	private:
		AssetDefinition* m_AssetPtr;

		friend class AssetRegistry;
		friend class AssetDefinition;

	public:
		struct Hasher
		{
			size_t operator()(const Asset& asset) const noexcept
			{
				return THash<void*>()(asset.m_AssetPtr);
			}
		};
	};

	// Asset class inline implementation

	inline AssetDefinition* Asset::operator->() const
	{
		AssetDefinition* def = GetAssetDefinition();
		ionverify(def, "Cannot dereference an asset handle of a non-existing asset. {{{}}}", (void*)m_AssetPtr);
		return def;
	}

	inline AssetDefinition& Asset::operator*() const
	{
		AssetDefinition* def = GetAssetDefinition();
		ionverify(def, "Cannot dereference an asset handle of a non-existing asset. {{{}}}", (void*)m_AssetPtr);
		return *def;
	}

	inline bool Asset::IsValid() const
	{
		return m_AssetPtr;
	}

	inline Asset::operator bool() const
	{
		return IsValid();
	}

	inline bool Asset::operator==(const Asset& other) const
	{
		return m_AssetPtr == other.m_AssetPtr;
	}

	inline bool Asset::operator!=(const Asset& other) const
	{
		return m_AssetPtr != other.m_AssetPtr;
	}

	class ION_API AssetImporter
	{
	public:
		static void ImportColladaMeshAsset(const TShared<AssetFileMemoryBlock>& block, MeshAssetData& outData);
		static void ImportImageAsset(const TShared<AssetFileMemoryBlock>& block, Image& outImage);
	};
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
