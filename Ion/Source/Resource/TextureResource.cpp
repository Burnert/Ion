#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

#include "Core/Asset/AssetRegistry.h"
#include "Core/File/XML.h"

namespace Ion
{
	TResourcePtr<TextureResource> TextureResource::Query(const Asset& asset)
	{
		return Resource::Query<TextureResource>(asset);
	}

	bool TextureResource::IsLoaded() const
	{
		return m_RenderData.IsAvailable();
	}

	static ETextureFilteringMethod ParseFilterString(char* csFilter)
	{
		if (strcmp(csFilter, "Linear") == 0)
			return ETextureFilteringMethod::Linear;
		if (strcmp(csFilter, "Nearest") == 0)
			return ETextureFilteringMethod::Nearest;

		return (ETextureFilteringMethod)0xFF;
	}

	bool TextureResource::ParseAssetFile(const FilePath& path, GUID& outGuid, TextureResourceDescription& outDescription)
	{
		String assetDefinition;
		File::ReadToString(path, assetDefinition);

		TUnique<XMLDocument> xml = MakeUnique<XMLDocument>(assetDefinition);

		// <IonAsset>
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		IASSET_CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset, path);

		// <TextureResource>
		XMLNode* nodeTextureResource = nodeIonAsset->first_node(IASSET_NODE_TextureResource);
		ionexcept(nodeTextureResource,
			"Asset \"%s\" cannot be used as a Texture Resource.\n"
			"Node <" IASSET_NODE_TextureResource "> not found.\n",
			StringConverter::WStringToString(path.ToString()).c_str())
			return false;

		// guid=
		XMLAttribute* texResource_attrGuid = nodeTextureResource->first_attribute(IASSET_ATTR_guid);
		IASSET_CHECK_ATTR(texResource_attrGuid, IASSET_ATTR_guid, IASSET_NODE_TextureResource, path);

		String sGuid = texResource_attrGuid->value();
		outGuid = GUID(sGuid);
		ionexcept(outGuid, "Invalid GUID.")
			return false;

		outDescription.Properties = { };

		// <Properties>
		XMLNode* nodeProperties = nodeTextureResource->first_node(IASSET_NODE_Properties);
		if (nodeProperties)
		{
			// <Filter>
			XMLNode* nodeFilter = nodeProperties->first_node(IASSET_NODE_TextureResource_Prop_Filter);
			if (nodeFilter)
			{
				// value=
				XMLAttribute* filter_attrValue = nodeFilter->first_attribute(IASSET_ATTR_value);
				IASSET_CHECK_ATTR(filter_attrValue, IASSET_ATTR_value, IASSET_NODE_TextureResource_Prop_Filter, path);

				char* csFilter = filter_attrValue->value();
				ETextureFilteringMethod filter = ParseFilterString(csFilter);
				// 0xFF means the value could not be parsed
				ionexcept(filter != (ETextureFilteringMethod)0xFF, "Invalid filtering mode. (\"%s\")",
					StringConverter::WStringToString(path.ToString()).c_str())
					return false;

				outDescription.Properties.Filter = filter;
			}
			else
			{
				outDescription.Properties.Filter = ETextureFilteringMethod::Default;
			}
		}

		return true;
	}
}
