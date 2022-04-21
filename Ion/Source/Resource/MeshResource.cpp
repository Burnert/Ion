#include "IonPCH.h"

#include "MeshResource.h"
#include "ResourceManager.h"

#include "Core/Asset/AssetRegistry.h"
#include "Core/File/XML.h"

namespace Ion
{
	TShared<MeshResource> MeshResource::Query(const Asset& asset)
	{
		return Resource::Query<MeshResource>(asset);
	}

	bool MeshResource::IsLoaded() const
	{
		return m_RenderData.IsAvailable();
	}

	bool MeshResource::ParseAssetFile(const FilePath& path, GUID& outGuid, MeshResourceDescription& outDescription)
	{
		String assetDefinition;
		File::ReadToString(path, assetDefinition);

		TUnique<XMLDocument> xml = MakeUnique<XMLDocument>(assetDefinition);

		// <IonAsset>
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		IASSET_CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset, path);

		// <MeshResource>
		XMLNode* nodeMeshResource = nodeIonAsset->first_node(IASSET_NODE_MeshResource);
		ionexcept(nodeMeshResource,
			"Asset \"%s\" cannot be used as a Mesh Resource.\n"
			"Node <" IASSET_NODE_MeshResource "> not found.\n",
			StringConverter::WStringToString(path.ToString()).c_str())
			return false;

		// guid=
		XMLAttribute* meshResource_attrGuid = nodeMeshResource->first_attribute(IASSET_ATTR_guid);
		IASSET_CHECK_ATTR(meshResource_attrGuid, IASSET_ATTR_guid, IASSET_NODE_MeshResource, path);

		String sGuid = meshResource_attrGuid->value();
		outGuid = GUID(sGuid);
		ionexcept(outGuid, "Invalid GUID.")
			return false;

		return true;
	}
}
