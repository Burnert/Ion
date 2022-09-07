#pragma once

#include "AssetCommon.h"

namespace Ion
{
	class ION_API AssetRegistry
	{
	public:
		/**
		 * @brief K - String - stores the virtual path of an asset.
		 * V - AssetDefinition - asset representation in memory.
		 */
		using AssetMap = THashMap<String, AssetDefinition>;

		static IAssetType* FindType(const String& name);
		
		static IAssetType& RegisterType(std::unique_ptr<IAssetType>&& customAssetType);

		/**
		 * @brief Registers an asset. Creates an AssetDefinition
		 * in the asset collection using the specified initializer.
		 * 
		 * @param initializer Asset description
		 * @return Reference to the asset definition
		 */
		static AssetDefinition& Register(const AssetInitializer& initializer);

		/**
		 * @brief Removes the asset definition object from the registry.
		 * 
		 * @param asset AssetDefinition object to remove
		 */
		static void Unregister(const AssetDefinition& asset);

		/**
		 * @brief Find an asset definition by virtual path.
		 *
		 * @param virtualPath a virtual path to an asset (e.g. "[Engine]/Materials/DefaultMaterial")
		 * @return If found, the AssetDefinition pointer, else nullptr
		 */
		static AssetDefinition* Find(const String& virtualPath);

		/**
		 * @brief Checks if the asset with a specified virtual path is registered.
		 * 
		 * @param virtualPath Virtual path to check
		 */
		static bool IsRegistered(const String& virtualPath);

		/**
		 * @brief Checks if the asset with a specified handle is registered.
		 * 
		 * @param asset Asset handle
		 */
		static bool IsRegistered(const Asset& asset);

		/**
		 * @brief Checks if the asset definition pointer is valid
		 * 
		 * @param asset AssetDefinition pointer
		 */
		static bool IsValid(AssetDefinition* asset);

		/**
		 * @brief Creates an array of handles to all the registered assets.
		 */
		static TArray<Asset> GetAllRegisteredAssets();

		/**
		 * @brief Creates an array of handles to all the registered assets of single type.
		 * 
		 * @param type Asset type
		 */
		static TArray<Asset> GetAllRegisteredAssets(const IAssetType& type);

		/**
		 * @brief Get the Assets Map with AssetDefinition objects
		 * 
		 * @return Const Asset Map reference
		 */
		static const AssetMap& GetAssetsMap();

		/**
		 * @brief Scans the Engine content directory for .iasset files and registers them.
		 */
		static void RegisterEngineAssets();

		static void RegisterEngineVirtualRoots(const FilePath& content, const FilePath& shaders);

		static void RegisterVirtualRoot(const String& root, const FilePath& physicalPath);

		static void RegisterAssetsInVirtualRoot(const String& virtualRoot);

		static const FilePath& ResolveVirtualRoot(const String& virtualRoot);

		static bool IsVirtualRootRegistered(const String& virtualRoot);

	private:
		AssetRegistry();

		static AssetRegistry& Get();

	private:
		AssetMap m_Assets;
		THashSet<AssetDefinition*> m_AssetPtrs;

		THashMap<String, std::unique_ptr<IAssetType>> m_AssetTypes;

		THashMap<String, FilePath> m_VirtualRoots;
	};

	// AssetRegistry class inline implementation ------------------------------

	inline const AssetRegistry::AssetMap& AssetRegistry::GetAssetsMap()
	{
		return Get().m_Assets;
	}
}
