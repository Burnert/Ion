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
	enum class EMaterialParameterType
	{
		Null = 0,
		Scalar,
		Vector,
		Texture2D
	};

#define _MPTFSHelper(type) if (strcmp(str, #type) == 0) return EMaterialParameterType::type
	inline static EMaterialParameterType MaterialParameterTypeFromString(const char* str)
	{
		_MPTFSHelper(Scalar);
		_MPTFSHelper(Vector);
		_MPTFSHelper(Texture2D);
		return EMaterialParameterType::Null;
	}

	using TMaterialParameterTypeVariant = TVariant<float, Vector4, GUID>;
}
