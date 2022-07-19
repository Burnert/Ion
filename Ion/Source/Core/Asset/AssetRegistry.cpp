#include "IonPCH.h"

#include "AssetRegistry.h"
#include "Asset.h"

#include "Core/Task/EngineTaskQueue.h"
#include "Core/File/Collada.h"

#include "Application/EnginePath.h"

#define CHECK_NODE(node, nodeName) IASSET_CHECK_NODE(node, nodeName, m_AssetDefinitionPath)
#define CHECK_ATTR(attr, attrName, nodeName) IASSET_CHECK_ATTR(attr, attrName, nodeName, m_AssetDefinitionPath)

namespace Ion
{
	// AssetDefinition ----------------------------------------------------------------

	AssetDefinition::AssetDefinition(const AssetInitializer& initializer) :
		m_Guid(initializer.Guid),
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
		return Asset(m_Guid);
	}

	AssetDefinition& AssetRegistry::Register(const AssetInitializer& initializer)
	{
		AssetRegistry& instance = Get();

		auto it = instance.m_Assets.find(initializer.Guid);
		if (it != instance.m_Assets.end())
		{
			LOG_ERROR("Cannot register the asset. An asset with the same GUID {{{0}}} already exists.", initializer.Guid);
			return it->second;
		}

		auto& [guid, assetDef] = *Get().m_Assets.emplace(initializer.Guid, AssetDefinition(initializer)).first;
		return assetDef;
	}

	void AssetRegistry::Unregister(const AssetDefinition& asset)
	{
		AssetRegistry& instance = Get();

		const GUID& guid = asset.GetGuid();

		ionassert(instance.m_Assets.find(guid) != instance.m_Assets.end(),
			"Asset \"%ls\" does not exist in the registry.", asset.GetDefinitionPath().ToString().c_str());

		instance.m_Assets.erase(guid);
	}

	AssetDefinition* AssetRegistry::Find(const GUID& guid)
	{
		AssetRegistry& instance = Get();

		auto it = instance.m_Assets.find(guid);
		if (it == instance.m_Assets.end())
		{
			return nullptr;
		}

		return &it->second;
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
			Asset asset = AssetFinder(assetNode->Get().FullPath).Resolve();
			LOG_TRACE(L"Loaded Engine Asset \"{0}\".", asset->GetDefinitionPath().ToString());
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
		m_WorkQueue(EngineTaskQueue::Get())
	{
	}

	AssetRegistry& AssetRegistry::Get()
	{
		static AssetRegistry* c_Instance = new AssetRegistry;
		return *c_Instance;
	}
}
