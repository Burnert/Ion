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

		static constexpr const char* FileExtension = ".iasset";
		static constexpr const char* FileExtensionNoDot = FileExtension + 1;

		/**
		 * @brief An Asset Handle initialized with a nullptr, that cannot serve as a None handle.
		 * Cannot be dereferenced.
		 */
		static const Asset InvalidHandle;
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
		 * @brief Is the Asset handle valid (can be null)
		 */
		bool IsValid() const;

		bool IsNone() const;

		bool IsAccessible() const;

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

		static bool IsVirtualRoot(const StringView& root);
		static bool IsStandardVirtualRoot(const StringView& root);
		static bool IsValidVirtualPath(const String& virtualPath);
		static String GetRootOfVirtualPath(const String& virtualPath);
		static String GetRestOfVirtualPath(const String& virtualPath);

	private:
		explicit Asset(AssetDefinition* asset);
		struct InvalidInitializerT { };
		Asset(InvalidInitializerT);

		static bool Parse(AssetInitializer& inOutInitializer);

		static Result<Asset, IOError, FileNotFoundError> RegisterAsset(const FilePath& path, const String& virtualPath);

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
		ionverify(def, "Cannot dereference an asset handle of a non-existing asset. {{{}}}", (void*)m_AssetPtr.Get());
		return def;
	}

	inline AssetDefinition& Asset::operator*() const
	{
		AssetDefinition* def = GetAssetDefinition();
		ionverify(def, "Cannot dereference an asset handle of a non-existing asset. {{{}}}", (void*)m_AssetPtr.Get());
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

	inline bool Asset::IsAccessible() const
	{
		return m_AssetPtr && !IsNone();
	}

	inline Asset::operator bool() const
	{
		return IsAccessible();
	}

	inline bool Asset::operator==(const Asset& other) const
	{
		return (m_AssetPtr == other.m_AssetPtr) || (IsNone() && other.IsNone());
	}

	inline bool Asset::operator!=(const Asset& other) const
	{
		return !operator==(other);
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
