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
			LOG_ERROR("Cannot register the asset. An asset with the same virtual path \"{0}\" already exists.", initializer.VirtualPath);
			return it->second;
		}

		auto& [vp, assetDef] = *instance.m_Assets.emplace(initializer.VirtualPath, AssetDefinition(initializer)).first;
		instance.m_AssetPtrs.emplace(&assetDef);
		return assetDef;
	}

	void AssetRegistry::Unregister(const AssetDefinition& asset)
	{
		AssetRegistry& instance = Get();

		const String& virtualPath = asset.GetVirtualPath();

		ionassert(instance.m_Assets.find(virtualPath) != instance.m_Assets.end(),
			"Asset \"{0}\" does not exist in the registry.", StringConverter::WStringToString(asset.GetDefinitionPath().ToString()));

		instance.m_AssetPtrs.erase(&instance.m_Assets.at(virtualPath));
		instance.m_Assets.erase(virtualPath);
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

		return IsValid(asset.m_AssetPtr.Get());
	}

	bool AssetRegistry::IsValid(AssetDefinition* asset)
	{
		AssetRegistry& instance = Get();

		return instance.m_AssetPtrs.find(asset) != instance.m_AssetPtrs.end();
	}

	void AssetRegistry::RegisterEngineAssets()
	{
		AssetRegistry& instance = Get();

		FilePath engineContentPath = EnginePath::GetEngineContentPath();

		TShared<TTreeNode<FileInfo>> engineContent = engineContentPath.Tree();

		TArray<TTreeNode<FileInfo>*> assets = engineContent->FindAllNodesRecursiveDF([](FileInfo& fileInfo)
		{
			File file(fileInfo.Filename);
			WString extension = file.GetExtension();
			return extension == L"iasset";
		});

		for (TTreeNode<FileInfo>*& assetNode : assets)
		{
			// Get the relative path
			FilePath relativePath = FilePath(assetNode->Get().FullPath).AsRelativeFrom(EnginePath::GetEngineContentPath());

			// Remove the extension
			WString last = relativePath.LastElement();
			last = last.substr(0, last.rfind(L".iasset"));
			relativePath.Back();
			relativePath.ChangeDirectory(last);

			// Make a virtual path string
			String virtualPath = "[Engine]/" + StringConverter::WStringToString(relativePath.ToString());

			// Register the asset
			Asset asset = Asset::Resolve(virtualPath).Unwrap();
			LOG_TRACE(L"Registered Engine Asset \"{0}\".", asset->GetDefinitionPath().ToString());
		}
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

	AssetRegistry& AssetRegistry::Get()
	{
		static AssetRegistry* c_Instance = new AssetRegistry;
		return *c_Instance;
	}
}
