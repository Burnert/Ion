#include "IonPCH.h"

#include "AssetParser.h"
#include "AssetRegistry.h"

namespace Ion
{
	AssetParser::AssetParser(const Asset& asset) :
		m_Parser(asset->GetDefinitionPath()),
		m_Asset(asset)
	{
		ionassert(m_Asset);
	}

	AssetParser& AssetParser::BeginAsset()
	{
		ionassert(!m_Parser.IsOpen(), "Cannot open the asset while it's already open.");

		m_Parser.Open()
			.EnterNode(IASSET_NODE_IonAsset);
		return *this;
	}

	AssetParser& AssetParser::ParseInfo(EAssetType& outType, GUID& outGuid)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_IonAsset);

		m_Parser.ParseAttributes(IASSET_NODE_Info,
			IASSET_ATTR_type, [&outType](const XMLParser::Interface& iface, String type)
			{
				outType = ParseAssetTypeString(type);
				if (outType == EAssetType::Invalid)
					iface.SendFail("Invalid asset type.");
			},
			IASSET_ATTR_guid, [&outGuid](const XMLParser::Interface& iface, String guid)
			{
				outGuid = GUID(guid);
				if (!outGuid)
					iface.SendFail("Invalid asset GUID.");
			}
		);
		return *this;
	}

	AssetParser& AssetParser::ParseName(String& outName)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_IonAsset);

		m_Parser.ParseNodeValue(IASSET_NODE_Name,
			[&outName](String name) { outName.swap(name); });
		return *this;
	}

	AssetParser& AssetParser::ExpectType(EAssetType type)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_IonAsset);

		m_Parser.ExpectAttributes(IASSET_NODE_Info,
			IASSET_ATTR_type, [type](String sType)
			{
				return type == ParseAssetTypeString(sType);
			});
		return *this;
	}

	XMLParserResult AssetParser::Finalize()
	{
		return m_Parser.Finalize();
	}

	MaterialAssetParser::MaterialAssetParser(const Asset& asset) :
		AssetParser(asset)
	{
	}

	MaterialAssetParser& MaterialAssetParser::BeginAsset()
	{
		EAssetType type;
		GUID guid(GUID::Zero);
		String name;

		AssetParser::BeginAsset()
			.ExpectType(EAssetType::Material)
			.ParseInfo(type, guid)
			.ParseName(name);

		return *this;
	}

	MaterialAssetParser& MaterialAssetParser::BeginMaterial()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_IonAsset);

		m_Parser.EnterNode(IASSET_NODE_Material);
		return *this;
	}

	MaterialAssetParser& MaterialAssetParser::EndMaterial()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Material);

		m_Parser.ExitNode();
		return *this;
	}

	MaterialAssetParser::ParameterValues MaterialAssetParser::ParseParameterValues(EMaterialParameterType type)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Material_Parameter);

		auto parseValue = [this, type](const String& defValue) -> TMaterialParameterTypeVariant
		{
			switch (type)
			{
				case EMaterialParameterType::Scalar:
				{
					TOptional<float> value = ParseFloatString(defValue.c_str());

					if (!value)
					{
						String message = fmt::format("Cannot parse the scalar parameter value string. \"{0}\" -> float", defValue);
						m_Parser.GetInterface().SendError(message);
						return 0.0f;
					}
					return *value;
				}
				case EMaterialParameterType::Vector:
				{
					TOptional<Vector4> value = ParseVector4String(defValue.c_str());

					if (!value)
					{
						String message = fmt::format("Cannot parse the vector parameter value string. \"{0}\" -> Vector4", defValue);
						m_Parser.GetInterface().SendError(message);
						return Vector4(0.0f);
					}
					return *value;
				}
				case EMaterialParameterType::Texture2D:
				{
					TOptional<GUID> value = ParseGuidString(defValue.c_str());

					if (!value)
					{
						String message = fmt::format("Cannot parse the texture2D parameter value string. \"{0}\" -> GUID", defValue);
						m_Parser.GetInterface().SendError(message);
						return GUID::Zero;
					}
					return *value;
				}
			}
			m_Parser.GetInterface().SendError("Tried to parse values of an invalid material parameter type.");
			return 0.0f;
		};

		ParameterValues values;

		m_Parser
			.TryParseNodeValue(IASSET_NODE_Material_Parameter_Default, [&](String def) { values.Default = parseValue(def); })
			.TryParseNodeValue(IASSET_NODE_Material_Parameter_Min,     [&](String min) { values.Min     = parseValue(min); })
			.TryParseNodeValue(IASSET_NODE_Material_Parameter_Max,     [&](String max) { values.Max     = parseValue(max); });

		return values;
	}
}
