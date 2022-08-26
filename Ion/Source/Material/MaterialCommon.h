#pragma once

#define IASSET_NODE_Material                           "Material"
#define IASSET_NODE_Material_Code                      "Code"
#define IASSET_NODE_Material_Parameter                 "Parameter"
#define IASSET_NODE_Material_Parameter_Default         "Default"
#define IASSET_NODE_Material_Parameter_Min             "Min"
#define IASSET_NODE_Material_Parameter_Max             "Max"

#define IASSET_NODE_MaterialInstance                   "MaterialInstance"
#define IASSET_NODE_MaterialInstance_ParameterInstance "ParameterInstance"

namespace Ion
{
	REGISTER_LOGGER(MaterialLogger, "Material");

	enum class EMaterialParameterType
	{
		Null = 0,
		Scalar,
		Vector,
		Texture2D
	};

	template<>
	struct TEnumParser<EMaterialParameterType>
	{
		ENUM_PARSER_TO_STRING_BEGIN(EMaterialParameterType)
		ENUM_PARSER_TO_STRING_HELPER(Null)
		ENUM_PARSER_TO_STRING_HELPER(Scalar)
		ENUM_PARSER_TO_STRING_HELPER(Vector)
		ENUM_PARSER_TO_STRING_HELPER(Texture2D)
		ENUM_PARSER_TO_STRING_END()

		ENUM_PARSER_FROM_STRING_BEGIN(EMaterialParameterType)
		ENUM_PARSER_FROM_STRING_HELPER(Null)
		ENUM_PARSER_FROM_STRING_HELPER(Scalar)
		ENUM_PARSER_FROM_STRING_HELPER(Vector)
		ENUM_PARSER_FROM_STRING_HELPER(Texture2D)
		ENUM_PARSER_FROM_STRING_END()
	};

#define _MPTFSHelper(type) if (strcmp(str, #type) == 0) return EMaterialParameterType::type
	inline static EMaterialParameterType MaterialParameterTypeFromString(const char* str)
	{
		_MPTFSHelper(Scalar);
		_MPTFSHelper(Vector);
		_MPTFSHelper(Texture2D);
		return EMaterialParameterType::Null;
	}

	using TMaterialParameterTypeVariant = TVariant<float, Vector4, String>;

	struct MaterialAssetParameterValues
	{
		TMaterialParameterTypeVariant Default;
		TMaterialParameterTypeVariant Min;
		TMaterialParameterTypeVariant Max;
	};

	struct MaterialInstanceAssetParameterInstanceValues
	{
		TMaterialParameterTypeVariant Value;
	};
}
