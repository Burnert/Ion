#include "IonPCH.h"

#include "MeshResource.h"
#include "ResourceManager.h"

#include "Core/Asset/AssetRegistry.h"
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

	static bool ParseDefaults(XMLNode* nodeDefaults, MeshResourceDefaults& outDefaults, const FilePath& path)
	{
		// <Texture>
		XMLNode* nodeTexture = nodeDefaults->first_node(IASSET_NODE_Defaults_Texture);
		if (nodeTexture)
		{
			XMLAttribute* texture_attrAsset = nodeTexture->first_attribute(IASSET_ATTR_asset);
			IASSET_CHECK_ATTR(texture_attrAsset, IASSET_ATTR_asset, IASSET_NODE_Defaults_Texture, path);

			String sAsset = texture_attrAsset->value();
			GUID assetGuid(sAsset);
			outDefaults.TextureAsset = Asset::Find(assetGuid);
		}
		return true;
	}

	bool MeshResource::ParseAssetFile(const FilePath& path, GUID& outGuid, MeshResourceDescription& outDescription)
	{
		String assetDefinition;
		File::ReadToString(path, assetDefinition);

		TUnique<XMLDocument> xml = MakeUnique<XMLDocument>(assetDefinition);

		// <IonAsset>
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		IASSET_CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset, path);

		// <Resource>
		XMLNode* nodeResource = nodeIonAsset->first_node(IASSET_NODE_Resource);
		IASSET_CHECK_NODE(nodeResource, IASSET_NODE_Resource, path);

		// <Mesh>
		XMLNode* nodeMeshResource = nodeResource->first_node(IASSET_NODE_Resource_Mesh);
		ionexcept(nodeMeshResource,
			"Asset \"%s\" cannot be used as a Mesh Resource.\n"
			"Node <" IASSET_NODE_Resource_Mesh "> not found.\n",
			StringConverter::WStringToString(path.ToString()).c_str())
			return false;

		// guid=
		XMLAttribute* meshResource_attrGuid = nodeMeshResource->first_attribute(IASSET_ATTR_guid);
		IASSET_CHECK_ATTR(meshResource_attrGuid, IASSET_ATTR_guid, IASSET_NODE_Resource_Mesh, path);

		String sGuid = meshResource_attrGuid->value();
		outGuid = GUID(sGuid);
		ionexcept(outGuid, "Invalid GUID.")
			return false;

		// <Defaults>
		XMLNode* nodeDefaults = nodeMeshResource->first_node(IASSET_NODE_Defaults);
		if (nodeDefaults)
		{
			// Check if the resource has any default settings
			ionexcept(ParseDefaults(nodeDefaults, outDescription.Defaults, path), "Could not parse the <Defaults> node.")
				return false;
		}

		return true;
	}
}
