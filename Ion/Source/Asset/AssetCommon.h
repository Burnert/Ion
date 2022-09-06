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

#pragma region Asset Type abstract base class

	class AssetType;

	class IAssetCustomData
	{
	public:
		virtual AssetType& GetType() const = 0;
	};

#define REGISTER_ASSET_TYPE_CLASS(T) inline T& AT_##T = static_cast<T&>(AssetRegistry::RegisterType(std::make_unique<T>()))

	class AssetType
	{
	public:
		explicit AssetType(const String& typeName);

		/**
		 * @brief Asset type object. Handles parsing/exporting the specific asset type.
		 * 
		 * @details This function is called when registering the asset file.
		 * On a successful parse, it returns a custom object, derived from IAssetCustomData,
		 * which is a representation of the parsed data. It can later be cast to get
		 * the data (e.g. when creating a mesh / texture).
		 * 
		 * @return Pointer to IAssetCustomData with the parsed data or IOError on error.
		 */
		virtual Result<TSharedPtr<IAssetCustomData>, IOError> Parse(const std::shared_ptr<XMLDocument>& xml) = 0;

		const String& GetName() const;

		bool operator==(const AssetType& other) const;
		bool operator!=(const AssetType& other) const;

		size_t GetHash() const;

	private:
		String m_Name;
	};

	FORCEINLINE AssetType::AssetType(const String& typeName) :
		m_Name(typeName)
	{
	}

	FORCEINLINE const String& AssetType::GetName() const
	{
		return m_Name;
	}

	FORCEINLINE bool AssetType::operator==(const AssetType& other) const
	{
		return m_Name == other.m_Name;
	}

	FORCEINLINE bool AssetType::operator!=(const AssetType& other) const
	{
		return m_Name != other.m_Name;
	}

	FORCEINLINE size_t AssetType::GetHash() const
	{
		return THash<String>()(m_Name);
	}
#pragma endregion

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
		std::shared_ptr<XMLDocument> AssetXML;
		String VirtualPath;
		FilePath AssetDefinitionPath;

		/**
		 * @brief Construct a null Asset Initializer
		 */
		AssetInitializer(const std::shared_ptr<XMLDocument>& xml, const String& virtualPath, const FilePath& assetDefinitionPath) :
			AssetXML(xml),
			VirtualPath(virtualPath),
			AssetDefinitionPath(assetDefinitionPath)
		{
		}
	};
}
