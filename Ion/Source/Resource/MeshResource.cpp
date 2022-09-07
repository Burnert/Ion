#include "IonPCH.h"

#include "MeshResource.h"
#include "ResourceManager.h"

#include "Core/File/XML.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

namespace Ion
{
	Result<TSharedPtr<IAssetCustomData>, IOError> MeshAssetType::Parse(const std::shared_ptr<XMLDocument>& xml) const
	{
		TSharedPtr<MeshAssetData> data = MakeShared<MeshAssetData>();

		XMLParserResult result = AssetParser(xml)
			.BeginAsset(AT_MeshAssetType)
			.Begin(IASSET_NODE_Resource) // <Resource>
			.Begin(IASSET_NODE_Resource_Mesh) // <Mesh>
			.ParseCurrentAttributeTyped(IASSET_ATTR_guid, data->ResourceGuid)
			.TryEnterNode(IASSET_NODE_Defaults, [&data](AssetParser& parser) // <Defaults>
			{
				parser.EnterEachNode(IASSET_NODE_Defaults_Material, [&data](AssetParser& parser)
				{
					uint32 index = (uint32)-1;
					Asset asset;
					parser.ParseCurrentAttributeTyped(IASSET_ATTR_index, index);
					parser.ParseCurrentAttributes(IASSET_ATTR_asset, [&](String sAsset)
					{
						asset = Asset::Resolve(sAsset)
							.Err([&](auto& err) { ResourceLogger.Error("Could not set Mesh material to \"{}\".", sAsset); })
							.UnwrapOr(Asset::None);
					});

					if (index != (uint32)-1)
					{
						if (index >= data->Description.Defaults.MaterialAssets.size())
							data->Description.Defaults.MaterialAssets.resize(index + 1);
						data->Description.Defaults.MaterialAssets[index] = asset;
					}
				});
			}) // </Defaults>
			.End() // </Mesh>
			.End() // </Resource>
			.Finalize();

		if (!result.OK())
		{
			result.PrintMessages();
			ionthrow(IOError, result.GetFailMessage());
		}
		return data;
	}

	TSharedPtr<MeshResource> MeshResource::Query(const Asset& asset)
	{
		return Resource::Query<MeshResource>(asset);
	}

	bool MeshResource::IsLoaded() const
	{
		return m_RenderData.IsAvailable();
	}
}
