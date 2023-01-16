#include "IonPCH.h"

#include "MeshResource.h"
#include "ResourceManager.h"

#include "Core/File/XML.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

namespace Ion
{
	Result<void, IOError> MeshAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");
		ionassert(!inOutCustomData || inOutCustomData->GetType() == AT_MeshAssetType);

		TSharedPtr<MeshAssetData> data = inOutCustomData ? PtrCast<MeshAssetData>(inOutCustomData) : MakeShared<MeshAssetData>();

		XMLArchiveAdapter xmlAr = ar;
		ArchiveNode rootNode = ar.EnterRootNode();

		ArchiveNode nodeResource = ar.EnterNode(rootNode, "Resource", EArchiveNodeType::Map);

		xmlAr.EnterNode(IASSET_NODE_Resource);

		ArchiveNode nodeMesh = ar.EnterNode(nodeResource, "Mesh", EArchiveNodeType::Map);

		xmlAr.EnterNode(IASSET_NODE_Resource_Mesh);

		ArchiveNode nodeGuid = ar.EnterNode(nodeMesh, "Guid", EArchiveNodeType::Value);

		xmlAr.EnterAttribute(IASSET_ATTR_guid);
		nodeGuid &= data->ResourceGuid;
		xmlAr.ExitAttribute(); // IASSET_ATTR_guid

		ArchiveNode nodeDefaults = ar.EnterNode(nodeMesh, "Defaults", EArchiveNodeType::Map);

		if (xmlAr.TryEnterNode(IASSET_NODE_Defaults) || IS_YAML_AR(ar))
		{
			auto LSerializeMaterial = [&](int32 index = -1, ArchiveNode node = ArchiveNode())
			{
				ionassert(!ar.IsSaving() || index > -1);

				ArchiveNode nodeIndex = ar.EnterNode(node, "Index", EArchiveNodeType::Value);

				xmlAr.EnterAttribute(IASSET_ATTR_index);
				nodeIndex &= index;
				xmlAr.ExitAttribute(); // IASSET_ATTR_index

				ArchiveNode nodeAsset = ar.EnterNode(node, "Asset", EArchiveNodeType::Value);

				xmlAr.EnterAttribute(IASSET_ATTR_asset);
				String sAsset = ar.IsSaving() ? data->Description.Defaults.MaterialAssets[index]->GetVirtualPath() : EmptyString;
				nodeAsset &= sAsset;
				xmlAr.ExitAttribute(); // IASSET_ATTR_asset

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

			ArchiveNode nodeMaterials = ar.EnterNode(nodeDefaults, "Materials", EArchiveNodeType::Seq);

			if (ar.IsLoading())
			{
				ON_YAML_AR(ar)
				{
					ArchiveNode nodeMaterial = ar.EnterNode(nodeMaterials, "", EArchiveNodeType::Map);
					for (; nodeMaterial; nodeMaterial = ar.EnterNextNode(nodeMaterial, EArchiveNodeType::Map))
						LSerializeMaterial(-1, nodeMaterial);
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
					ON_YAML_AR(ar)
					{
						ArchiveNode nodeMaterial = ar.EnterNode(nodeMaterials, "", EArchiveNodeType::Map);
						LSerializeMaterial(i, nodeMaterial);
					}
					else
					{
						xmlAr.EnterNode(IASSET_NODE_Defaults_Material);
						LSerializeMaterial(i);
						xmlAr.ExitNode(); // IASSET_NODE_Defaults_Material
					}
				}
			}

			xmlAr.ExitNode(); // IASSET_NODE_Defaults
		}

		xmlAr.ExitNode(); // IASSET_NODE_Resource_Mesh

		xmlAr.ExitNode(); // IASSET_NODE_Resource

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
