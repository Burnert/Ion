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

	Result<void, IOError> MeshAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");
		ionassert(!inOutCustomData || inOutCustomData->GetType() == AT_MeshAssetType);

		TSharedPtr<MeshAssetData> data = inOutCustomData ? PtrCast<MeshAssetData>(inOutCustomData) : MakeShared<MeshAssetData>();

		XMLArchiveAdapter xmlAr = ar;

		ON_YAML_AR(ar) yml->EnterNode("Resource");

		xmlAr.EnterNode(IASSET_NODE_Resource);

		ON_YAML_AR(ar) yml->EnterNode("Mesh");

		xmlAr.EnterNode(IASSET_NODE_Resource_Mesh);

		ON_YAML_AR(ar) yml->EnterNode("Guid");

		xmlAr.EnterAttribute(IASSET_ATTR_guid);
		xmlAr << data->ResourceGuid;
		xmlAr.ExitAttribute(); // IASSET_ATTR_guid

		ON_YAML_AR(ar) yml->ExitNode();

		ON_YAML_AR(ar) yml->EnterNode("Defaults");

		if (xmlAr.TryEnterNode(IASSET_NODE_Defaults) || IS_YAML_AR(ar))
		{
			auto LSerializeMaterial = [&](int32 index = -1)
			{
				ionassert(!ar.IsSaving() || index > -1);

				ON_YAML_AR(ar) yml->EnterNode("Index");

				xmlAr.EnterAttribute(IASSET_ATTR_index);
				xmlAr << index;
				xmlAr.ExitAttribute(); // IASSET_ATTR_index

				ON_YAML_AR(ar) yml->ExitNode();

				ON_YAML_AR(ar) yml->EnterNode("Asset");

				xmlAr.EnterAttribute(IASSET_ATTR_asset);
				String sAsset = ar.IsSaving() ? data->Description.Defaults.MaterialAssets[index]->GetVirtualPath() : EmptyString;
				xmlAr << sAsset;
				xmlAr.ExitAttribute(); // IASSET_ATTR_asset

				ON_YAML_AR(ar) yml->ExitNode();

				if (ar.IsLoading())
				{
					Asset asset = Asset::Resolve(sAsset)
						.Err([&](Error& err) { ResourceLogger.Error("Could not set Mesh material to \"{}\".\n{}", sAsset, err.Message); })
						.UnwrapOr(Asset::None);

					if (index >= data->Description.Defaults.MaterialAssets.size())
						data->Description.Defaults.MaterialAssets.resize(index + 1);
					data->Description.Defaults.MaterialAssets[index] = asset;
				}
			};

			ON_YAML_AR(ar) yml->EnterNode("Materials");

			ON_YAML_AR(ar) yml->BeginSeq();

			if (ar.IsLoading())
			{
				ON_YAML_AR(ar)
				{
					while (yml->IterateSeq())
						LSerializeMaterial();
				}
				else
				{
					for (bool b = xmlAr.TryEnterNode(IASSET_NODE_Defaults_Material); b || (xmlAr.ExitNode(), 0); b = xmlAr.TryEnterSiblingNode())
						LSerializeMaterial();
				}
			}
			else if (ar.IsSaving())
			{
				for (int32 i = 0; i < data->Description.Defaults.MaterialAssets.size(); ++i)
				{
					ON_YAML_AR(ar) yml->IterateSeq();

					xmlAr.EnterNode(IASSET_NODE_Defaults_Material);
					LSerializeMaterial(i);
					xmlAr.ExitNode(); // IASSET_NODE_Defaults_Material
				}
			}

			ON_YAML_AR(ar) yml->EndSeq();

			ON_YAML_AR(ar) yml->ExitNode();
			
			xmlAr.ExitNode(); // IASSET_NODE_Defaults
		}

		ON_YAML_AR(ar) yml->ExitNode();

		xmlAr.ExitNode(); // IASSET_NODE_Resource_Mesh

		ON_YAML_AR(ar) yml->ExitNode();

		xmlAr.ExitNode(); // IASSET_NODE_Resource

		ON_YAML_AR(ar) yml->ExitNode();

		inOutCustomData = data;

		return Ok();
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
