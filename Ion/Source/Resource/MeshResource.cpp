#include "IonPCH.h"

#include "MeshResource.h"
#include "ResourceManager.h"

#include "Core/Asset/AssetRegistry.h"
#include "Core/Asset/AssetParser.h"
#include "Core/File/XML.h"

namespace Ion
{
	TResourcePtr<MeshResource> MeshResource::Query(const Asset& asset)
	{
		return Resource::Query<MeshResource>(asset);
	}

	bool MeshResource::IsLoaded() const
	{
		return m_RenderData.IsAvailable();
	}

	bool MeshResource::ParseAssetFile(const Asset& asset, GUID& outGuid, MeshResourceDescription& outDescription)
	{
		return AssetParser(asset)
			.BeginAsset(EAssetType::Mesh)
			.Begin(IASSET_NODE_Resource) // <Resource>
			.Begin(IASSET_NODE_Resource_Mesh) // <Mesh>
			.ParseCurrentAttributeTyped(IASSET_ATTR_guid, outGuid)
			.TryEnterNode(IASSET_NODE_Defaults, [&outDescription](AssetParser& parser) // <Defaults>
			{
				parser.EnterEachNode(IASSET_NODE_Defaults_Material, [&outDescription](AssetParser& parser)
				{
					uint32 index = (uint32)-1;
					Asset asset = Asset::InvalidHandle;
					parser.ParseCurrentAttributeTyped(IASSET_ATTR_index, index);
					parser.ParseCurrentAttributes(IASSET_ATTR_asset, [&](String sAsset) { asset = Asset::Resolve(sAsset); });

					if (index != (uint32)-1 && asset.IsValid())
					{
						if (index >= outDescription.Defaults.MaterialAssets.size())
							outDescription.Defaults.MaterialAssets.resize(index + 1);
						outDescription.Defaults.MaterialAssets[index] = Move(asset);
					}
				});
			}) // </Defaults>
			.End() // </Mesh>
			.End() // </Resource>
			.Finalize()
			.OK();
	}
}
