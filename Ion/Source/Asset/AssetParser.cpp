#include "IonPCH.h"

#include "AssetParser.h"
#include "AssetRegistry.h"
#include "AssetDefinition.h"

namespace Ion
{
	// Asset Parser Base ------------------------------------------------------------------------

	AssetParser::AssetParser(const Asset& asset) :
		AssetParser(asset->GetDefinitionPath())
	{
		ionassert(asset);
	}

	AssetParser::AssetParser(const FilePath& assetPath) :
		XMLParser(assetPath)
	{
		ionassert(EqualsCI(assetPath.GetExtension(), StringView(Asset::FileExtension)));
	}

	AssetParser::AssetParser(const std::shared_ptr<XMLDocument>& xml) :
		XMLParser(xml)
	{
	}

	AssetParser& AssetParser::BeginAsset()
	{
		Open();
		EnterNode(IASSET_NODE_IonAsset);
		return *this;
	}

	AssetParser& AssetParser::BeginAsset(AssetType& type)
	{
		Open();
		EnterNode(IASSET_NODE_IonAsset);
		ExpectType(type);
		return *this;
	}

	AssetParser& AssetParser::BeginAsset(const String& type)
	{
		ionassert(AssetRegistry::FindType(type), "An asset type with name \"{}\" does not exist.", type);
		return BeginAsset(*AssetRegistry::FindType(type));
	}

	AssetParser& AssetParser::Begin(const String& nodeName)
	{
		EnterNode(nodeName);
		return *this;
	}

	AssetParser& AssetParser::End()
	{
		ExitNode();
		return *this;
	}

	AssetParser& AssetParser::ParseInfo(AssetType*& outType, GUID& outGuid)
	{
		ionassert(GetCurrentNodeName() == IASSET_NODE_IonAsset);

		ParseAttributes(IASSET_NODE_Info,
			IASSET_ATTR_type, [&outType](const XMLParser::MessageInterface& iface, String sType)
			{
				outType = AssetRegistry::FindType(sType);
				if (!outType)
					iface.SendFail("Invalid asset type.");
			},
			IASSET_ATTR_guid, [&outGuid](const XMLParser::MessageInterface& iface, String guid)
			{
				GUID::FromString(guid)
					.Ok([&](const GUID& g) { outGuid = g; })
					.Err<StringConversionError>([&](auto& err) { iface.SendFail("Invalid asset GUID."); });
			}
		);
		return *this;
	}

	AssetParser& AssetParser::ParseName(String& outName)
	{
		ionassert(GetCurrentNodeName() == IASSET_NODE_IonAsset);

		if (CheckNode(IASSET_NODE_Name))
		{
			ParseNodeValue(IASSET_NODE_Name,
				[&outName](String name) { outName.swap(name); });
		}
		else
		{
			outName = GetPath().LastElement();
		}
		return *this;
	}

	AssetParser& AssetParser::ExpectType(AssetType& type)
	{
		ionassert(GetCurrentNodeName() == IASSET_NODE_IonAsset);

		ExpectAttributes(IASSET_NODE_Info,
			IASSET_ATTR_type, [&type](String sType)
			{
				AssetType* type = AssetRegistry::FindType(sType);
				return type && type->GetName() == sType;
			});
		return *this;
	}
}
