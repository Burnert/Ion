#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

namespace Ion
{
	AssetType& ImageAssetData::GetType() const
	{
		return AT_ImageAssetType;
	}

	Result<TSharedPtr<IAssetCustomData>, IOError> ImageAssetType::Parse(const std::shared_ptr<XMLDocument>& xml)
	{
		TSharedPtr<ImageAssetData> data = MakeShared<ImageAssetData>();

		XMLParserResult result = AssetParser(xml)
			.BeginAsset(AT_ImageAssetType)
			.Begin(IASSET_NODE_Resource) // <Resource>
			.Begin(IASSET_NODE_Resource_Texture) // <Texture>
			.ParseCurrentAttributeTyped(IASSET_ATTR_guid, data->ResourceGuid)
			.TryEnterNode(IASSET_NODE_Properties, [&data](AssetParser& parser) // <Defaults>
			{
				parser.TryEnterNode(IASSET_NODE_Resource_Texture_Prop_Filter, [&data](AssetParser& parser)
				{
					ETextureFilteringMethod filter = ETextureFilteringMethod::Default;
					parser.ParseCurrentEnumAttribute(IASSET_ATTR_value, filter);

					data->Description.Properties.Filter = filter;
				});
			}) // </Defaults>
			.End() // </Texture>
			.End() // </Resource>
			.Finalize();

		if (!result.OK())
		{
			result.PrintMessages();
			ionthrow(IOError, result.GetFailMessage());
		}
		return data;
	}

	TSharedPtr<TextureResource> TextureResource::Query(const Asset& asset)
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

		ionbreak("Unknown filter type.");
		return (ETextureFilteringMethod)0xFF;
	}
}
