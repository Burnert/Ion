#include "IonPCH.h"

#include "AssetRegistry.h"
#include "Asset.h"

#include "Core/Task/EngineTaskQueue.h"
#include "Core/File/Collada.h"

namespace Ion
{
	// AssetDefinition ----------------------------------------------------------------

	AssetDefinition::AssetDefinition(const AssetInitializer& initializer) :
		m_Guid(initializer.Guid),
		m_AssetDefinitionPath(initializer.AssetDefinitionPath),
		m_AssetReferencePath(initializer.AssetReferencePath),
		m_Type(initializer.Type),
		m_bImportExternal(initializer.bImportExternal),
		m_bIsLoaded(false),
		m_AssetData(EAssetType::None)
	{
		ParseAssetDefinitionFile();
	}

	AssetDefinition::~AssetDefinition()
	{
	}

	void AssetDefinition::ParseAssetDefinitionFile()
	{
		// @TODO: Parse the rest of the file here
	}

	// FAssetLoadWork -----------------------------------------------

	void FAssetLoadWork::Schedule()
	{
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
		auto it = Get().m_Assets.find(initializer.Guid);
		if (it != Get().m_Assets.end())
		{
			LOG_ERROR("Cannot register the asset. An asset with the same GUID {{{0}}} already exists.", initializer.Guid);
			return it->second;
		}

		auto& [guid, assetDef] = *Get().m_Assets.emplace(initializer.Guid, AssetDefinition(initializer)).first;
		return assetDef;
	}

	AssetDefinition* AssetRegistry::Find(const GUID& guid)
	{
		auto it = Get().m_Assets.find(guid);
		if (it == Get().m_Assets.end())
		{
			LOG_WARN("Cannot find asset with GUID {{{0}}}.", guid.ToString());
			return nullptr;
		}

		return &it->second;
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
