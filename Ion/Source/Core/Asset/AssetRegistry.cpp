#include "IonPCH.h"

#include "AssetRegistry.h"
#include "Asset.h"

#include "Core/Task/EngineTaskQueue.h"
#include "Core/File/Collada.h"

#include "Application/EnginePath.h"

#define ASSET_REGISTRY_ASSET_MAP_BUCKETS 256

#define CHECK_NODE(node, nodeName) IASSET_CHECK_NODE(node, nodeName, m_AssetDefinitionPath)
#define CHECK_ATTR(attr, attrName, nodeName) IASSET_CHECK_ATTR(attr, attrName, nodeName, m_AssetDefinitionPath)

namespace Ion
{
	// AssetDefinition ----------------------------------------------------------------

	AssetDefinition::AssetDefinition(const AssetInitializer& initializer) :
		m_Guid(initializer.Guid),
		m_VirtualPath(initializer.VirtualPath),
		m_AssetDefinitionPath(initializer.AssetDefinitionPath),
		m_AssetReferencePath(initializer.AssetReferencePath),
		m_Type(initializer.Type),
		m_bImportExternal(initializer.bImportExternal),
		m_AssetData(EAssetType::None),
		m_Info({ })
	{
		ParseAssetDefinitionFile(initializer.IAssetXML);
	}

	AssetDefinition::~AssetDefinition()
	{
	}

	bool AssetDefinition::ParseAssetDefinitionFile(const TShared<XMLDocument>& xml)
	{
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset);

		XMLNode* nodeName = nodeIonAsset->first_node(IASSET_NODE_Name);
		m_Info.Name = nodeName ?
			nodeName->value() :
			StringConverter::WStringToString(m_AssetDefinitionPath.LastElement());

		XMLNode* nodeResource = nodeIonAsset->first_node(IASSET_NODE_Resource);
		if (nodeResource)
		{
			XMLNode* nodeCurrentResource = nodeResource->first_node();
			while (nodeCurrentResource)
			{
				m_Info.ResourceUsage.push_back(nodeCurrentResource->name());
				nodeCurrentResource = nodeCurrentResource->next_sibling();
			}
		}

		return true;
	}

	// FAssetLoadWork -----------------------------------------------

	void FAssetLoadWork::Schedule()
	{
#if ION_DEBUG
		m_DebugName = StringConverter::WStringToString(AssetPath.ToString());
#endif
		// Executes on a worker thread
		Execute = [*this](IMessageQueueProvider& queue)
		{
			ionassert(OnLoad);
			ionassert(OnError);

			ionassert(AssetPath.IsFile());
			ionassert(AssetType != EAssetType::Invalid);

			File assetFile(AssetPath, EFileMode::Read | EFileMode::DoNotOpen);

			AssetData assetData(AssetType);

			// @TODO: Use the memory pool here instead of normal allocations

			switch (AssetType)
			{
				case EAssetType::Mesh:
				{
					String collada;
					File::ReadToString(AssetPath, collada);

					// @TODO: Refactor the ColladaDocument class a bit
					TUnique<ColladaDocument> colladaDoc = MakeUnique<ColladaDocument>(collada);
					const ColladaData& colladaData = colladaDoc->GetData();

					TShared<MeshAssetData> mesh = MakeShared<MeshAssetData>();
					mesh->Layout = colladaData.Layout;
					
					mesh->Vertices.Ptr = new float[colladaData.VertexAttributeCount];
					mesh->Vertices.Count = colladaData.VertexAttributeCount;

					mesh->Indices.Ptr = new uint32[colladaData.IndexCount];
					mesh->Indices.Count = colladaData.IndexCount;

					memcpy(mesh->Vertices.Ptr, colladaData.VertexAttributes, colladaData.VertexAttributeCount * sizeof(float));
					memcpy(mesh->Indices.Ptr, colladaData.Indices, colladaData.IndexCount * sizeof(uint32));

					assetData.Variant = mesh;
					break;
				}
				case EAssetType::Image:
				{
					TShared<Image> image = MakeShared<Image>();
					image->Load(assetFile);
					if (!image->IsLoaded())
					{
						queue.PushMessage(FTaskMessage([*this]
						{
							OnError();
						}));
					}
					assetData.Variant = image;
					break;
				}
			}

			queue.PushMessage(FTaskMessage([*this, assetData]
			{
				OnLoad(assetData);
			}));
		};

		AssetRegistry::ScheduleWork(*this);
	}

	// AssetRegistry ----------------------------------------------------------------

	Asset AssetDefinition::GetHandle() const
	{
		return Asset(const_cast<AssetDefinition*>(this));
	}

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

		const String& vp = asset.GetVirtualPath();

		ionassert(instance.m_Assets.find(vp) != instance.m_Assets.end(),
			"Asset \"%ls\" does not exist in the registry.", asset.GetDefinitionPath().ToString().c_str());

		instance.m_AssetPtrs.erase(&instance.m_Assets.at(vp));
		instance.m_Assets.erase(vp);
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

		return IsRegistered(asset.m_AssetPtr.Get());
	}

	bool AssetRegistry::IsRegistered(AssetDefinition* asset)
	{
		AssetRegistry& instance = Get();

		return instance.m_AssetPtrs.find(asset) != instance.m_AssetPtrs.end();
	}

	void AssetRegistry::RegisterEngineAssets()
	{
		AssetRegistry& instance = Get();

		FilePath engineContentPath = EnginePath::GetEngineContentPath();

		instance.m_EngineContent = engineContentPath.Tree();

		TArray<TTreeNode<FileInfo>*> assets = instance.m_EngineContent->FindAllNodesRecursiveDF([](FileInfo& fileInfo)
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
			Asset asset = Asset::Resolve(virtualPath);
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
		m_WorkQueue(EngineTaskQueue::Get()),
		m_Assets(ASSET_REGISTRY_ASSET_MAP_BUCKETS)
	{
	}

	AssetRegistry& AssetRegistry::Get()
	{
		static AssetRegistry* c_Instance = new AssetRegistry;
		return *c_Instance;
	}
}
