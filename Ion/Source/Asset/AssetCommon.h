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

	class IAssetType;

	class IAssetCustomData
	{
	public:
		virtual IAssetType& GetType() const = 0;
	};

	// Implemented in AssetRegistry.cpp
	ION_API IAssetType& _RegisterType(std::unique_ptr<IAssetType>&& customAssetType);

#define REGISTER_ASSET_TYPE_CLASS(T) inline T& AT_##T = static_cast<T&>(_RegisterType(std::make_unique<T>()))
#define ASSET_TYPE_NAME_IMPL(name) \
	virtual const String& GetName() const override { \
		static String c_Name = name; \
		return c_Name; \
	}
#define ASSET_DATA_GETTYPE_IMPL(type) \
	virtual IAssetType& GetType() const override { \
		return type; \
	}

	class IAssetType
	{
	public:
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
		virtual Result<TSharedPtr<IAssetCustomData>, IOError> Parse(const std::shared_ptr<XMLDocument>& xml) const = 0;

		virtual Result<std::shared_ptr<XMLDocument>, IOError> Export(const TSharedPtr<IAssetCustomData>& data) const { ionthrow(IOError, "Export function not implemented."); }

		virtual Result<void, IOError> Serialize(Archive& ar, TSharedPtr<IAssetCustomData> customData) const { ionthrow(IOError, "Serialize function not implemented."); }

		virtual const String& GetName() const = 0;

		bool operator==(const IAssetType& other) const;
		bool operator!=(const IAssetType& other) const;

		size_t GetHash() const;
	};

	FORCEINLINE bool IAssetType::operator==(const IAssetType& other) const
	{
		return GetName() == other.GetName();
	}

	FORCEINLINE bool IAssetType::operator!=(const IAssetType& other) const
	{
		return GetName() != other.GetName();
	}

	FORCEINLINE size_t IAssetType::GetHash() const
	{
		return THash<String>()(GetName());
	}

#pragma endregion

	class AssetSerializer
	{
	public:
		FORCEINLINE static Result<void, IOError> EnterAssetAndSetCheckType(Archive& ar, IAssetType& type)
		{
			XMLArchiveAdapter xmlAr = ar;
			xmlAr.SeekRoot();

			xmlAr.EnterNode(IASSET_NODE_IonAsset);

			xmlAr.EnterNode(IASSET_NODE_Info);

			xmlAr.EnterAttribute(IASSET_ATTR_type);
			String sType = type.GetName();
			xmlAr << sType;

			xmlAr.ExitAttribute(); // IASSET_ATTR_type

			xmlAr.ExitNode(); // IASSET_NODE_Info

			if (type.GetName() == sType); else
			{
				ionthrow(IOError, "Wrong asset type.");
			}
			return Ok();
		}
	};

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
