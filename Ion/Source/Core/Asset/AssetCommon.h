#pragma once

#define IASSET_STR_INVALID_FILE "%s is not a valid Ion Asset file.\n\n"
#define IASSET_CHECK_NODE(node, nodeName, path) \
ionexcept(node, IASSET_STR_INVALID_FILE "<" nodeName "> node could not be found.\n", path.ToString()) return false
#define IASSET_CHECK_ATTR(attr, attrName, nodeName, path) \
ionexcept(attr, IASSET_STR_INVALID_FILE attrName " attribute could not be found in node<" nodeName ">.\n", path.ToString()) return false

#define IASSET_NODE_IonAsset            "IonAsset"
#define IASSET_NODE_Info                "Info"
#define IASSET_ATTR_Info_type           "type"
#define IASSET_ATTR_Info_guid           "guid"
#define IASSET_NODE_ImportExternal      "ImportExternal"
#define IASSET_ATTR_ImportExternal_path "path"

namespace Ion
{
	// Fwd

	// Asset.h
	class Asset;
	class AssetFinder;
	// AssetRegistry.h
	class AssetRegistry;
	class AssetDefinition;
	// AssetWorkQueue.h
	struct IMessageQueueProvider;
	struct IAssetWork;
	struct AssetLoadWork;
	struct AssetMessage;
	class AssetWorker;
	class AssetWorkQueue;

	/* Used in .iasset file (e.g. <Info type="Ion.Image">) */
	enum class EAssetType : uint8
	{
		None = 0,
		Image,
		Mesh,
		Invalid = 0xFF,
	};

	/**
	 * @brief Structure returned by the AssetDefinition class
	 * @see AssetDefinition::Load()
	 */
	struct AssetData
	{
		const void* Data;
		size_t Size;
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
}
