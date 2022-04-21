#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

#include "Core/File/XML.h"

namespace Ion
{
	TShared<TextureResource> TextureResource::Query(const Asset& asset)
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

	bool TextureResource::ParseAssetFile(const FilePath& path, TextureResourceDescription& outDescription)
	{
		String assetDefinition;
		File::ReadToString(path, assetDefinition);

		TUnique<XMLDocument> xml = MakeUnique<XMLDocument>(assetDefinition);

		// <IonAsset>
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		IASSET_CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset, path);

		// <TextureResource>
		XMLNode* nodeTextureResource = nodeIonAsset->first_node(IASSET_NODE_TextureResource);
		if (nodeTextureResource)
		{
			// <Filter>
			XMLNode* nodeFilter = nodeTextureResource->first_node(IASSET_NODE_TextureResource_Filter);
			if (nodeFilter)
			{
				// value=
				XMLAttribute* filter_attrValue = nodeFilter->first_attribute(IASSET_ATTR_value);
				IASSET_CHECK_ATTR(filter_attrValue, IASSET_ATTR_value, IASSET_NODE_TextureResource_Filter, path);

				char* csFilter = filter_attrValue->value();
				ETextureFilteringMethod filter = ParseFilterString(csFilter);
				// 0xFF means the value could not be parsed
				ionexcept(filter != (ETextureFilteringMethod)0xFF, "Invalid filtering mode. (\"%s\")",
					StringConverter::WStringToString(path.ToString()).c_str())
					return false;

				outDescription.Filter = filter;
			}
			else
			{
				outDescription.Filter = ETextureFilteringMethod::Default;
			}
		}

		return true;
	}
}
