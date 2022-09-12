#pragma once

// Asset Commons ----------------------------------------------------

#define IASSET_NODE_IonAsset             "IonAsset"
#define IASSET_NODE_Info                 "Info"
#define IASSET_NODE_ImportExternal       "ImportExternal"
#define IASSET_NODE_Name                 "Name"
#define IASSET_NODE_Guid                 "Guid"

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

	struct ImportedMeshData
	{
		TMemoryBlock<float> Vertices;
		TMemoryBlock<uint32> Indices;
		TRef<RHIVertexLayout> Layout;

		ImportedMeshData() :
			Vertices({ }),
			Indices({ })
		{
		}

		~ImportedMeshData()
		{
			Vertices.Free();
			Indices.Free();
		}

		ImportedMeshData(const ImportedMeshData&) = delete;
		ImportedMeshData(ImportedMeshData&&) = delete;
		ImportedMeshData& operator=(const ImportedMeshData&) = delete;
		ImportedMeshData& operator=(ImportedMeshData&&) = delete;
	};

	/**
	 * @brief Used internally to initialize an AssetDefinition object.
	 */
	struct AssetInitializer
	{
		class IAssetType* Type;
		std::shared_ptr<XMLDocument> AssetXML;
		String VirtualPath;
		FilePath AssetDefinitionPath;

		// Existing asset initializer
		AssetInitializer(const std::shared_ptr<XMLDocument>& xml, const String& virtualPath, const FilePath& assetDefinitionPath) :
			Type(nullptr),
			AssetXML(xml),
			VirtualPath(virtualPath),
			AssetDefinitionPath(assetDefinitionPath)
		{
		}

		// New asset initializer
		AssetInitializer(IAssetType* type, const String& virtualPath, const FilePath& assetDefinitionPath) :
			Type(type),
			AssetXML(nullptr),
			VirtualPath(virtualPath),
			AssetDefinitionPath(assetDefinitionPath)
		{
		}
	};
}
