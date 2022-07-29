#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

#include "Core/Asset/AssetRegistry.h"
#include "Core/Asset/AssetParser.h"
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

	bool TextureResource::ParseAssetFile(const Asset& asset, GUID& outGuid, TextureResourceDescription& outDescription)
	{
		return AssetParser(asset)
			.BeginAsset(EAssetType::Image)
			.Begin(IASSET_NODE_Resource) // <Resource>
			.Begin(IASSET_NODE_Resource_Texture) // <Texture>
			.ParseCurrentAttributeTyped(IASSET_ATTR_guid, outGuid)
			.TryEnterNode(IASSET_NODE_Properties, [&outDescription](AssetParser& parser) // <Defaults>
			{
				parser.TryEnterNode(IASSET_NODE_Resource_Texture_Prop_Filter, [&outDescription](AssetParser& parser)
				{
					ETextureFilteringMethod filter = ETextureFilteringMethod::Default;
					parser.ParseCurrentEnumAttribute(IASSET_ATTR_value, filter);

					outDescription.Properties.Filter = filter;
				});
			}) // </Defaults>
			.End() // </Texture>
			.End() // </Resource>
			.Finalize()
			.OK();
	}
}
