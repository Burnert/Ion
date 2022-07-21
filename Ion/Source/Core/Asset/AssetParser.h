#pragma once

#include "Asset.h"
#include "Core/File/XMLParser.h"

// @TODO: Out of module include
#include "Material/MaterialCommon.h"

namespace Ion
{
	// Asset Parser Base ------------------------------------------------------------------------

	class ION_API AssetParser
	{
	public:
		AssetParser(const Asset& asset);

		AssetParser& BeginAsset();

		AssetParser& ParseInfo(EAssetType& outType, GUID& outGuid);
		AssetParser& ParseName(String& outName);

		AssetParser& ExpectType(EAssetType type);

		XMLParserResult Finalize();

	protected:
		XMLParser m_Parser;
		Asset m_Asset;
	};

	// Material Asset Parser ------------------------------------------------------------------

	// MaterialAssetParser should only be used where Material.h is included.
	class ION_API MaterialAssetParser : public AssetParser
	{
	public:
		using FLoadCode = TFunction<bool(String)>;

		struct ParameterValues
		{
			TMaterialParameterTypeVariant Default;
			TMaterialParameterTypeVariant Min;
			TMaterialParameterTypeVariant Max;
		};

		MaterialAssetParser(const Asset& asset);

		MaterialAssetParser& BeginAsset();

		MaterialAssetParser& BeginMaterial(/* type blend shader */);
		MaterialAssetParser& EndMaterial();

		template<typename FLoad>
		MaterialAssetParser& LoadCode(FLoad loadFunc);
		template<typename FForEach>
		MaterialAssetParser& ParseParameters(FForEach forEachParam);

	private:
		ParameterValues ParseParameterValues(EMaterialParameterType type);
	};

	template<typename FLoad>
	inline MaterialAssetParser& MaterialAssetParser::LoadCode(FLoad loadFunc)
	{
		static_assert(TIsConvertibleV<FLoad, FLoadCode>);

		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Material);

		m_Parser.ParseAttributes(IASSET_NODE_Material_Code,
			IASSET_ATTR_source, [&loadFunc](const XMLParser::MessageInterface& iface, String source /* Shader Path */)
			{
				if (source.empty())
				{
					iface.SendError("Material source code path not specified.");
					return;
				}

				if (!loadFunc(source))
				{
					String message = fmt::format("Could not load Material source code. \"{0}\"", source);
					iface.SendError(message);
					return;
				}
			});

		return *this;
	}

	template<typename FForEach>
	inline MaterialAssetParser& MaterialAssetParser::ParseParameters(FForEach forEachParam)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_Material);

		m_Parser.EnterEachNode(IASSET_NODE_Material_Parameter, [this, &forEachParam](XMLParser& parser)
		{
			String paramName;
			EMaterialParameterType paramType = EMaterialParameterType::Null;

			parser.ParseCurrentAttributes(
				IASSET_ATTR_type, [&paramType, &parser](String type)
				{
					paramType = MaterialParameterTypeFromString(type.c_str());
					if (paramType == EMaterialParameterType::Null)
					{
						String message = fmt::format("Invalid Material Parameter type: \"{0}\"", type);
						parser.GetInterface().SendError(message);
					}
				},
				IASSET_ATTR_name, [&paramName](String name)
				{
					paramName.swap(name);
				});

			ParameterValues values = ParseParameterValues(paramType);
			forEachParam(paramName, paramType, values);
		});

		return *this;
	}

	// Material Instance Asset Parser ------------------------------------------------------------------

	class ION_API MaterialInstanceAssetParser : public AssetParser
	{
	public:
		struct ParameterInstanceValue
		{
			TMaterialParameterTypeVariant Value;
		};

		MaterialInstanceAssetParser(const Asset& asset);

		MaterialInstanceAssetParser& BeginAsset();

		template<typename FParse>
		MaterialInstanceAssetParser& BeginMaterialInstance(FParse parseFunc);
		MaterialInstanceAssetParser& EndMaterialInstance();

		template<typename FForEach>
		MaterialInstanceAssetParser& ParseParameterInstances(FForEach forEachPI);

	private:
		ParameterInstanceValue ParseParameterInstanceValue(EMaterialParameterType type);
	};

	template<typename FParse>
	inline MaterialInstanceAssetParser& MaterialInstanceAssetParser::BeginMaterialInstance(FParse parseFunc)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_IonAsset);

		m_Parser.EnterNode(IASSET_NODE_MaterialInstance)
			.ParseCurrentAttributes(
				IASSET_ATTR_parent, [&parseFunc](const XMLParser::MessageInterface& iface, String parent)
				{
					TOptional<GUID> guidOpt = ParseGuidString(parent.c_str());
					if (guidOpt)
					{
						parseFunc(Asset::Find(*guidOpt));
					}
					else
					{
						String message = fmt::format("Cannot parse the Material parent GUID string: \"{0}\" -> GUID", parent);
						iface.SendFail(message);
					}
				});

		return *this;
	}

	template<typename FForEach>
	inline MaterialInstanceAssetParser& MaterialInstanceAssetParser::ParseParameterInstances(FForEach forEachPI)
	{
		ionassert(m_Parser.GetCurrentNodeName() == IASSET_NODE_MaterialInstance);

		m_Parser.EnterEachNode(IASSET_NODE_MaterialInstance_ParameterInstance, [this, &forEachPI](XMLParser& parser)
		{
			String paramName;
			EMaterialParameterType paramType = EMaterialParameterType::Null;

			parser.ParseCurrentAttributes(
				IASSET_ATTR_type, [&paramType, &parser](String type)
				{
					paramType = MaterialParameterTypeFromString(type.c_str());
					if (paramType == EMaterialParameterType::Null)
					{
						String message = fmt::format("Invalid Material Parameter Instance type: \"{0}\"", type);
						parser.GetInterface().SendError(message);
					}
				},
				IASSET_ATTR_name, [&paramName](String name)
				{
					paramName.swap(name);
				});

			ParameterInstanceValue value = ParseParameterInstanceValue(paramType);
			forEachPI(paramName, paramType, value, parser.GetInterface());
		});

		return *this;
	}
}
