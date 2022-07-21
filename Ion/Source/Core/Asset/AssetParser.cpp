#include "IonPCH.h"

#include "AssetParser.h"
#include "AssetRegistry.h"

namespace Ion
{
	// Asset Parser Base ------------------------------------------------------------------------

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
			IASSET_ATTR_type, [&outType](const XMLParser::MessageInterface& iface, String type)
			{
				outType = ParseAssetTypeString(type);
				if (outType == EAssetType::Invalid)
					iface.SendFail("Invalid asset type.");
			},
			IASSET_ATTR_guid, [&outGuid](const XMLParser::MessageInterface& iface, String guid)
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

	// Material Asset Parser ------------------------------------------------------------------

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

	static TMaterialParameterTypeVariant _ParseParamValue(const String& val, EMaterialParameterType type, XMLParser& parser)
	{
		switch (type)
		{
			case EMaterialParameterType::Scalar:
			{
				TOptional<float> value = ParseFloatString(val.c_str());

				if (!value)
				{
					String message = fmt::format("Cannot parse the scalar parameter value string. \"{0}\" -> float", val);
					parser.GetInterface().SendError(message);
					return 0.0f;
				}
				return *value;
			}
			case EMaterialParameterType::Vector:
			{
				TOptional<Vector4> value = ParseVector4String(val.c_str());

				if (!value)
				{
					String message = fmt::format("Cannot parse the vector parameter value string. \"{0}\" -> Vector4", val);
					parser.GetInterface().SendError(message);
					return Vector4(0.0f);
				}
				return *value;
			}
			case EMaterialParameterType::Texture2D:
			{
				TOptional<GUID> value = ParseGuidString(val.c_str());

				if (!value)
				{
					String message = fmt::format("Cannot parse the texture2D parameter value string. \"{0}\" -> GUID", val);
					parser.GetInterface().SendError(message);
					return GUID::Zero;
				}
				return *value;
			}
		}
		parser.GetInterface().SendError("Tried to parse values of an invalid material parameter type.");
		return 0.0f;
	}

	MaterialAssetParser::ParameterValues MaterialAssetParser::ParseParameterValues(EMaterialParameterType type)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Material_Parameter);

		ParameterValues values;
		m_Parser
			.TryParseNodeValue(IASSET_NODE_Material_Parameter_Default, [&](String def) { values.Default = _ParseParamValue(def, type, m_Parser); })
			.TryParseNodeValue(IASSET_NODE_Material_Parameter_Min,     [&](String min) { values.Min     = _ParseParamValue(min, type, m_Parser); })
			.TryParseNodeValue(IASSET_NODE_Material_Parameter_Max,     [&](String max) { values.Max     = _ParseParamValue(max, type, m_Parser); });
		return values;
	}

	// Material Instance Asset Parser ------------------------------------------------------------------

	MaterialInstanceAssetParser::MaterialInstanceAssetParser(const Asset& asset) :
		AssetParser(asset)
	{
	}

	MaterialInstanceAssetParser& MaterialInstanceAssetParser::BeginAsset()
	{
		AssetParser::BeginAsset()
			.ExpectType(EAssetType::MaterialInstance);
		return *this;
	}

	MaterialInstanceAssetParser& MaterialInstanceAssetParser::EndMaterialInstance()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_MaterialInstance);

		m_Parser.ExitNode();
		return *this;
	}

	MaterialInstanceAssetParser::ParameterInstanceValue MaterialInstanceAssetParser::ParseParameterInstanceValue(EMaterialParameterType type)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_MaterialInstance_ParameterInstance);

		ParameterInstanceValue value;
		m_Parser.TryParseCurrentNodeValue([&](String val) { value.Value = _ParseParamValue(val, type, m_Parser); });
		return value;
	}

	// Mesh Asset Parser ----------------------------------------------------------------------

	MeshAssetParser::MeshAssetParser(const Asset& asset) :
		AssetParser(asset),
		m_bNoDefaults(false)
	{
	}

	MeshAssetParser& MeshAssetParser::BeginAsset()
	{
		AssetParser::BeginAsset()
			.ExpectType(EAssetType::Mesh);
		return *this;
	}

	MeshAssetParser& MeshAssetParser::BeginResource()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_IonAsset);

		m_Parser.EnterNode(IASSET_NODE_Resource);
		return *this;
	}

	MeshAssetParser& MeshAssetParser::EndResource()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Resource);

		m_Parser.ExitNode();
		return *this;
	}
	
	MeshAssetParser& MeshAssetParser::EndMesh()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Resource_Mesh);

		m_Parser.ExitNode();
		return *this;
	}

	MeshAssetParser& MeshAssetParser::BeginDefaults()
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Resource_Mesh);

		if (!m_Parser.CheckNode(IASSET_NODE_Defaults))
		{
			m_bNoDefaults = true;
			return *this;
		}

		m_Parser.EnterNode(IASSET_NODE_Defaults);
		return *this;
	}

	MeshAssetParser& MeshAssetParser::EndDefaults()
	{
		if (m_bNoDefaults)
		{
			return *this;
		}

		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Defaults);

		m_Parser.ExitNode();
		return *this;
	}
}
