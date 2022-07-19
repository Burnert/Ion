#pragma once

#define IASSET_STR_INVALID_FILE "%s is not a valid Ion Asset file.\n\n"
#define IASSET_CHECK_NODE(node, nodeName, path) \
ionexcept(node, IASSET_STR_INVALID_FILE "<" nodeName "> node has not ben found.\n", StringConverter::WStringToString(path.ToString()).c_str()) return false
#define IASSET_CHECK_ATTR(attr, attrName, nodeName, path) \
ionexcept(attr, IASSET_STR_INVALID_FILE attrName " attribute could not be found in node <" nodeName ">.\n", StringConverter::WStringToString(path.ToString()).c_str()) return false

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

// Resource Usage -----------------------------------------------------

#define IASSET_NODE_Resource          "Resource"
#define IASSET_NODE_Resource_Mesh     "Mesh"
#define IASSET_NODE_Resource_Texture  "Texture"

namespace Ion
{
	// Fwd

	// Asset.h
	class Asset;
	class AssetFinder;
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

	struct AssetInfo
	{
		String Name;
		TArray<String> ResourceUsage;
	};

	struct MeshAssetData
	{
		TMemoryBlock<float> Vertices;
		TMemoryBlock<uint32> Indices;
		TShared<RHIVertexLayout> Layout;

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
	 * @brief Structure returned by the AssetDefinition class
	 * @see AssetDefinition::Load()
	 */
	struct AssetData
	{
		TVariant<
			TShared<void>,
			TShared<Image>,
			TShared<MeshAssetData>
		// @TODO: Support a custom type
		> Variant;

		AssetData(EAssetType type);

		template<typename T>
		TShared<T> Get() const;

		EAssetType GetType() const;

		bool IsValid() const;
		operator bool() const;

	private:
		void Reset();

	private:
		EAssetType m_Type;
	};

	/**
	 * @brief Used internally to initialize an AssetDefinition object.
	 */
	struct AssetInitializer
	{
		TShared<XMLDocument> IAssetXML;

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
			Guid(GUID::Zero),
			AssetDefinitionPath(),
			AssetReferencePath(),
			Type(EAssetType::None),
			bImportExternal(false)
		{
		}
	};

	// AssetData class inline implementation

	inline AssetData::AssetData(EAssetType type) :
		m_Type(type)
	{
		ionassert(type != EAssetType::Invalid);

		if (type != EAssetType::None)
			Reset();
	}

	template<typename T>
	inline TShared<T> AssetData::Get() const
	{
		return VariantCast<TShared<T>>(Variant);
	}

	inline EAssetType AssetData::GetType() const
	{
		return m_Type;
	}

	inline bool AssetData::IsValid() const
	{
		return m_Type != EAssetType::None;
	}

	inline AssetData::operator bool() const
	{
		return IsValid();
	}

	inline void AssetData::Reset()
	{
		switch (m_Type)
		{
		case EAssetType::Image:
			Variant = TShared<Image>();
			break;
		case EAssetType::Mesh:
			Variant = TShared<MeshAssetData>();
			break;
		}
	}
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
