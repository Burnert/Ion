#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

namespace Ion
{
	Result<void, IOError> ImageAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");
		ionassert(!inOutCustomData || inOutCustomData->GetType() == AT_ImageAssetType);

		TSharedPtr<ImageAssetData> data = inOutCustomData ? PtrCast<ImageAssetData>(inOutCustomData) : MakeShared<ImageAssetData>();

		XMLArchiveAdapter xmlAr = ar;
		ArchiveNode nodeRoot = ar.EnterRootNode();

		ArchiveNode nodeResource = ar.EnterNode(nodeRoot, "Resource", EArchiveNodeType::Map);

		xmlAr.EnterNode(IASSET_NODE_Resource);

		ArchiveNode nodeTexture = ar.EnterNode(nodeResource, "Texture", EArchiveNodeType::Map);

		xmlAr.EnterNode(IASSET_NODE_Resource_Texture);

		ArchiveNode nodeGuid = ar.EnterNode(nodeTexture, "Guid", EArchiveNodeType::Value);

		xmlAr.EnterAttribute(IASSET_ATTR_guid);
		nodeGuid &= data->ResourceGuid;
		xmlAr.ExitAttribute(); // IASSET_ATTR_guid

		ArchiveNode nodeProperties = ar.EnterNode(nodeTexture, "Properties", EArchiveNodeType::Map);

		if (ar.IsLoading() && (xmlAr.TryEnterNode(IASSET_NODE_Properties) || IS_YAML_AR(ar)) ||
			ar.IsSaving())
		{
			ArchiveNode nodeFilter = ar.EnterNode(nodeProperties, "Filter", EArchiveNodeType::Value);

			if (xmlAr.TryEnterNode(IASSET_NODE_Resource_Texture_Prop_Filter) || IS_YAML_AR(ar))
			{
				xmlAr.EnterAttribute(IASSET_ATTR_value);
				nodeFilter &= data->Description.Properties.Filter;
				xmlAr.ExitAttribute();

				xmlAr.ExitNode(); // IASSET_NODE_Resource_Texture_Prop_Filter
			}

			xmlAr.ExitNode(); // IASSET_NODE_Properties
		}

		xmlAr.ExitNode(); // IASSET_NODE_Resource_Texture

		xmlAr.ExitNode(); // IASSET_NODE_Resource

		inOutCustomData = data;

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
