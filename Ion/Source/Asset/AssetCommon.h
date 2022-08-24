#pragma once

// Asset Commons ----------------------------------------------------

#define IASSET_NODE_IonAsset             "IonAsset"
#define IASSET_NODE_Info                 "Info"
#define IASSET_NODE_ImportExternal       "ImportExternal"
#define IASSET_NODE_Name                 "Name"

#define IASSET_ATTR_type                 "type"
#define IASSET_ATTR_path                 "path"
#define IASSET_ATTR_value                "value"
#define IASSET_ATTR_guid                 "guid"
#define IASSET_ATTR_name                 "name"
#define IASSET_ATTR_source               "source"
#define IASSET_ATTR_parent               "parent"
#define IASSET_ATTR_blend                "blend"
#define IASSET_ATTR_shader               "shader"
#define IASSET_ATTR_index                "index"

// Resource Usage -----------------------------------------------------

#define IASSET_NODE_Resource          "Resource"
#define IASSET_NODE_Resource_Mesh     "Mesh"
#define IASSET_NODE_Resource_Texture  "Texture"

namespace Ion
{
	REGISTER_LOGGER(AssetLogger, "Asset");

	// Fwd

	// Asset.h
	class Asset;
	// AssetRegistry.h
	class AssetRegistry;
	class AssetDefinition;

	/* Used in .iasset file (e.g. <Info type="Ion.Image">) */
	enum class EAssetType : uint8
	{
		None = 0,
		Generic,
		Image,
		Mesh,
		Data,
		Material,
		MaterialInstance,
		Invalid = 0xFF,
	};

	template<>
	struct TEnumParser<EAssetType>
	{
		ENUM_PARSER_TO_STRING_BEGIN(EAssetType)
		ENUM_PARSER_TO_STRING_HELPER(None)
		ENUM_PARSER_TO_STRING_HELPER(Generic)
		ENUM_PARSER_TO_STRING_HELPER(Image)
		ENUM_PARSER_TO_STRING_HELPER(Mesh)
		ENUM_PARSER_TO_STRING_HELPER(Data)
		ENUM_PARSER_TO_STRING_HELPER(Material)
		ENUM_PARSER_TO_STRING_HELPER(MaterialInstance)
		ENUM_PARSER_TO_STRING_HELPER(Invalid)
		ENUM_PARSER_TO_STRING_END()

		ENUM_PARSER_FROM_STRING_BEGIN(EAssetType)
		ENUM_PARSER_FROM_STRING_HELPER(None)
		ENUM_PARSER_FROM_STRING_HELPER(Generic)
		ENUM_PARSER_FROM_STRING_HELPER(Image)
		ENUM_PARSER_FROM_STRING_HELPER(Mesh)
		ENUM_PARSER_FROM_STRING_HELPER(Data)
		ENUM_PARSER_FROM_STRING_HELPER(Material)
		ENUM_PARSER_FROM_STRING_HELPER(MaterialInstance)
		ENUM_PARSER_FROM_STRING_HELPER(Invalid)
		ENUM_PARSER_FROM_STRING_END()
	};

	EAssetType ParseAssetTypeString(const String& sType);

	inline static TOptional<Vector4> ParseVector4String(const char* str)
	{
		Vector4 value;
		float* currentValue = (float*)&value;
		char* pEnd;
		// Is the currentValue still inside the Vector4
		while (currentValue - (float*)&value < 4)
		{
			*currentValue++ = strtof(str, &pEnd);
			if (pEnd == str || errno == ERANGE)
			{
				AssetLogger.Error("Invalid value.");
				return NullOpt;
			}
			// Omit the space between components;
			if (*pEnd)
				str = pEnd + 1;
		}
		return value;
	}

	using AssetFileMemoryBlock = TMemoryBlock<uint8>;

	struct AssetInfo
	{
		String Name;
		TArray<String> ResourceUsage;
	};

	struct MeshAssetData
	{
		TMemoryBlock<float> Vertices;
		TMemoryBlock<uint32> Indices;
		std::shared_ptr<RHIVertexLayout> Layout;

		MeshAssetData() :
			Vertices({ }),
			Indices({ })
		{
		}

		~MeshAssetData()
		{
			Vertices.Free();
			Indices.Free();
		}

		MeshAssetData(const MeshAssetData&) = delete;
		MeshAssetData(MeshAssetData&&) = delete;
		MeshAssetData& operator=(const MeshAssetData&) = delete;
		MeshAssetData& operator=(MeshAssetData&&) = delete;
	};

	/**
	 * @brief Used internally to initialize an AssetDefinition object.
	 */
	struct AssetInitializer
	{
		std::shared_ptr<XMLDocument> IAssetXML;

		String VirtualPath;
		GUID Guid;
		FilePath AssetDefinitionPath;
		FilePath AssetReferencePath;
		EAssetType Type;
		uint8 bImportExternal : 1;

		/**
		 * @brief Construct an invalid Asset Initializer
		 */
		static AssetInitializer Invalid()
		{
			AssetInitializer init;
			init.Type = EAssetType::Invalid;
			return init;
		}

		/**
		 * @brief Construct a null Asset Initializer
		 */
		AssetInitializer() :
			VirtualPath(),
			Guid(GUID::Zero),
			AssetDefinitionPath(),
			AssetReferencePath(),
			Type(EAssetType::None),
			bImportExternal(false)
		{
		}
	};
}

template<>
NODISCARD FORCEINLINE String ToString<Ion::EAssetType>(Ion::EAssetType value)
{
	switch (value)
	{
	case Ion::EAssetType::None:              return "None";
	case Ion::EAssetType::Generic:           return "Generic";
	case Ion::EAssetType::Mesh:              return "Mesh";
	case Ion::EAssetType::Image:             return "Image";
	case Ion::EAssetType::Data:              return "Data";
	case Ion::EAssetType::Material:          return "Material";
	case Ion::EAssetType::MaterialInstance:  return "MaterialInstance";
	case Ion::EAssetType::Invalid:           return "Invalid";
	}
	ionassert(0, "Invalid enum value.");
	return "";
}