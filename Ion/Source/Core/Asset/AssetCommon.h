#pragma once

#define IASSET_STR_INVALID_FILE "%s is not a valid Ion Asset file.\n\n"
#define IASSET_CHECK_NODE(node, nodeName, path) \
ionexcept(node, IASSET_STR_INVALID_FILE "<" nodeName "> node could not be found.\n", path.ToString()) return false
#define IASSET_CHECK_ATTR(attr, attrName, nodeName, path) \
ionexcept(attr, IASSET_STR_INVALID_FILE attrName " attribute could not be found in node<" nodeName ">.\n", path.ToString()) return false

// Asset Commons ----------------------------------------------------

#define IASSET_NODE_IonAsset             "IonAsset"
#define IASSET_NODE_Info                 "Info"
#define IASSET_ATTR_Info_type            "type"
#define IASSET_NODE_ImportExternal       "ImportExternal"
#define IASSET_ATTR_ImportExternal_path  "path"

#define IASSET_ATTR_value                "value"
#define IASSET_ATTR_guid                 "guid"

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
		Image,
		Mesh,
		Invalid = 0xFF,
	};

	struct MeshAssetData
	{
		TMemoryBlock<float> Vertices;
		TMemoryBlock<uint32> Indices;
		TShared<RHIVertexLayout> Layout;
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
	case Ion::EAssetType::None:    return "None";
	case Ion::EAssetType::Invalid: return "Invalid";
	case Ion::EAssetType::Mesh:    return "Mesh";
	case Ion::EAssetType::Image:   return "Image";
	}
	ionassert(0, "Invalid enum value.");
	return "";
}
