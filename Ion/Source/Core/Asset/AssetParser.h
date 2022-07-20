#pragma once

#include "Asset.h"
#include "Core/File/XMLParser.h"

// @TODO: Out of module include
#include "Material/MaterialCommon.h"

namespace Ion
{
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
			IASSET_ATTR_source, [&loadFunc](const XMLParser::Interface& iface, String source /* Shader Path */)
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
}
