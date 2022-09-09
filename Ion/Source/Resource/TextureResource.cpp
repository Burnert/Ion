#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

namespace Ion
{
	Result<TSharedPtr<IAssetCustomData>, IOError> ImageAssetType::Parse(const std::shared_ptr<XMLDocument>& xml) const
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

	Result<void, IOError> ImageAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData> customData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");

		TSharedPtr<ImageAssetData> data = PtrCast<ImageAssetData>(customData);

		XMLArchiveAdapter xmlAr = ar;
		xmlAr.SeekRoot();

		xmlAr.EnterNode(IASSET_NODE_IonAsset);

		xmlAr.EnterNode(IASSET_NODE_Info);

		xmlAr.EnterAttribute(IASSET_ATTR_type);
		String type = AT_ImageAssetType.GetName();
		xmlAr << type;
		if (AT_ImageAssetType.GetName() == type); else
		{
			ionthrow(IOError, "Wrong asset type.");
		}
		xmlAr.ExitAttribute(); // IASSET_ATTR_type

		xmlAr.ExitNode(); // IASSET_NODE_Info

		xmlAr.EnterNode(IASSET_NODE_Resource);
		xmlAr.EnterNode(IASSET_NODE_Resource_Texture);

		xmlAr.EnterAttribute(IASSET_ATTR_guid);
		xmlAr << data->ResourceGuid;
		xmlAr.ExitAttribute(); // IASSET_ATTR_guid

		if (ar.IsLoading() && xmlAr.TryEnterNode(IASSET_NODE_Properties) ||
			ar.IsSaving())
		{
			if (xmlAr.TryEnterNode(IASSET_NODE_Resource_Texture_Prop_Filter))
			{
				xmlAr.EnterAttribute(IASSET_ATTR_value);
				xmlAr << data->Description.Properties.Filter;
				xmlAr.ExitAttribute();

				xmlAr.ExitNode(); // IASSET_NODE_Resource_Texture_Prop_Filter
			}

			xmlAr.ExitNode(); // IASSET_NODE_Properties
		}

		xmlAr.ExitNode(); // IASSET_NODE_Resource_Texture
		xmlAr.ExitNode(); // IASSET_NODE_Resource

		xmlAr.ExitNode(); // IASSET_NODE_IonAsset

		return Ok();
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
