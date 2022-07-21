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
		XMLParserResult result = MeshAssetParser(asset)
			.BeginAsset() // <IonAsset>
			.BeginResource() // <Resource>
			.BeginMesh([&outGuid](GUID& guid) { outGuid.Swap(guid); }) // <Mesh>
			.BeginDefaults() // <Defaults>
			.ParseMaterials([&outDescription](uint32 index, const Asset& asset) // Each <Material>
			{
				if (index >= outDescription.Defaults.MaterialAssets.size())
					outDescription.Defaults.MaterialAssets.resize(index + 1);
				outDescription.Defaults.MaterialAssets[index] = Move(asset);
			})
			.EndDefaults() // </Defaults>
			.EndMesh() // </Mesh>
			.EndResource() // </Resource>
			.Finalize(); // </IonAsset>

		return result.OK();
	}
}
