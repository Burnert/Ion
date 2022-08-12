#include "IonPCH.h"

#include "AssetRegistry.h"
#include "AssetDefinition.h"
#include "Asset.h"

#include "Application/EnginePath.h"

#define ASSET_REGISTRY_ASSET_MAP_BUCKETS 256

namespace Ion
{
	// AssetRegistry ----------------------------------------------------------------

	AssetDefinition& AssetRegistry::Register(const AssetInitializer& initializer)
	{
		AssetRegistry& instance = Get();

		ionassert(!initializer.VirtualPath.empty(), "Asset cannot have an empty virtual path.");

		auto it = instance.m_Assets.find(initializer.VirtualPath);
		if (it != instance.m_Assets.end())
		{
			AssetLogger.Error("Cannot register the asset. An asset with the same virtual path \"{0}\" already exists.", initializer.VirtualPath);
			return it->second;
		}

		auto& [vp, assetDef] = *instance.m_Assets.emplace(initializer.VirtualPath, AssetDefinition(initializer)).first;
		instance.m_AssetPtrs.emplace(&assetDef);

		AssetLogger.Info("Registered asset \"{0}\".", assetDef.GetVirtualPath());

		return assetDef;
	}

	void AssetRegistry::Unregister(const AssetDefinition& asset)
	{
		AssetRegistry& instance = Get();

		const String& virtualPath = asset.GetVirtualPath();

		ionassert(instance.m_Assets.find(virtualPath) != instance.m_Assets.end(),
			"Asset \"{0}\" does not exist in the registry.", virtualPath);

		instance.m_AssetPtrs.erase(&instance.m_Assets.at(virtualPath));
		instance.m_Assets.erase(virtualPath);

		AssetLogger.Info("Unregistered asset \"{0}\".", virtualPath);
	}

	AssetDefinition* AssetRegistry::Find(const String& virtualPath)
	{
		AssetRegistry& instance = Get();

		if (virtualPath.empty())
			return nullptr;

		auto it = instance.m_Assets.find(virtualPath);
		if (it == instance.m_Assets.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	bool AssetRegistry::IsRegistered(const String& virtualPath)
	{
		return (bool)Find(virtualPath);
	}

	bool AssetRegistry::IsRegistered(const Asset& asset)
	{
		if (!asset)
			return false;

		AssetRegistry& instance = Get();

		return IsValid(asset.m_AssetPtr);
	}

	bool AssetRegistry::IsValid(AssetDefinition* asset)
	{
		AssetRegistry& instance = Get();

		return instance.m_AssetPtrs.find(asset) != instance.m_AssetPtrs.end();
	}

	void AssetRegistry::RegisterEngineAssets()
	{
		AssetRegistry& instance = Get();

		AssetLogger.Info("Registering Engine Assets...");

		instance.RegisterAssetsInVirtualRoot(Asset::VirtualRoot::Engine);

		AssetLogger.Info("Registered Engine Assets.");
	}

	void AssetRegistry::RegisterEngineVirtualRoots()
	{
		AssetLogger.Info("Registering Engine Virtual Roots...");

		AssetRegistry::RegisterVirtualRoot(Asset::VirtualRoot::Engine, EnginePath::GetEngineContentPath());
		AssetRegistry::RegisterVirtualRoot(Asset::VirtualRoot::Shaders, EnginePath::GetShadersPath());
		// @TODO: AssetRegistry::RegisterVirtualRoot(Asset::VirtualRoot::Game, X);

		AssetLogger.Info("Registered Engine Virtual Roots.");
	}

	void AssetRegistry::RegisterVirtualRoot(const String& root, const FilePath& physicalPath)
	{
		ionassert(Asset::IsVirtualRoot(root));
		ionassert(!IsVirtualRootRegistered(root), "Virtual root already registered.");

		AssetRegistry& instance = Get();
		
		FilePath fixedPath = physicalPath.Fix();

		instance.m_VirtualRoots.emplace(root, fixedPath);

		AssetLogger.Info("Registered asset virtual root: \"{}\" -> \"{}\"", root, fixedPath.ToString());
	}

	void AssetRegistry::RegisterAssetsInVirtualRoot(const String& virtualRoot)
	{
		ionassert(Asset::IsVirtualRoot(virtualRoot));
		ionassert(IsVirtualRootRegistered(virtualRoot));

		AssetRegistry& instance = Get();

		FilePath rootDir = instance.ResolveVirtualRoot(virtualRoot);

		AssetLogger.Info("Registering Assets in Virtual Root \"{}\" -> \"{}\"...", virtualRoot, rootDir.ToString());

		TShared<TTreeNode<FileInfo>> content = rootDir.Tree();

		TArray<TTreeNode<FileInfo>*> assets = content->FindAllNodesRecursiveDF([](FileInfo& fileInfo)
		{
			return EqualsCI(FilePath(fileInfo.Filename).GetExtension(), StringView(Asset::FileExtension));
		});

		for (TTreeNode<FileInfo>*& assetNode : assets)
		{
			// Get the relative path
			FilePath relativePath = FilePath(assetNode->Get().FullPath).RelativeTo(rootDir);

			// Remove the extension
			String last = relativePath.LastElement();
			last = last.substr(0, last.rfind(Asset::FileExtension));
			relativePath.Back();
			String sRelative = relativePath.ToString();

			// Make a virtual path string
			String virtualPath = fmt::format("{}/{}/{}", virtualRoot, sRelative, last);

			// Register the asset
			Asset asset = Asset::RegisterAsset(assetNode->Get().FullPath, virtualPath).Unwrap();
		}
	}

	const FilePath& AssetRegistry::ResolveVirtualRoot(const String& virtualRoot)
	{
		AssetRegistry& instance = Get();

		ionverify(IsVirtualRootRegistered(virtualRoot), "Virtual root not registered.");

		return instance.m_VirtualRoots.at(virtualRoot);
	}

	TArray<Asset> AssetRegistry::GetAllRegisteredAssets()
	{
		AssetRegistry& instance = Get();

		TArray<Asset> assets;

		for (auto& [guid, asset] : instance.m_Assets)
		{
			assets.emplace_back(asset.GetHandle());
		}

		return assets;
	}

	TArray<Asset> AssetRegistry::GetAllRegisteredAssets(EAssetType type)
	{
		AssetRegistry& instance = Get();

		TArray<Asset> assets;

		for (auto& [guid, asset] : instance.m_Assets)
		{
			if (asset.GetType() == type)
			{
				assets.emplace_back(asset.GetHandle());
			}
		}

		return assets;
	}

	AssetRegistry::AssetRegistry() :
		m_Assets(ASSET_REGISTRY_ASSET_MAP_BUCKETS),
		m_AssetPtrs(ASSET_REGISTRY_ASSET_MAP_BUCKETS)
	{
	}

	bool AssetRegistry::IsVirtualRootRegistered(const String& virtualRoot)
	{
		ionassert(Asset::IsVirtualRoot(virtualRoot), "Invalid virtual root string.");

		AssetRegistry& instance = Get();

		return instance.m_VirtualRoots.find(virtualRoot) != instance.m_VirtualRoots.end();
	}

	AssetRegistry& AssetRegistry::Get()
	{
		static AssetRegistry* c_Instance = new AssetRegistry;
		return *c_Instance;
	}
}
