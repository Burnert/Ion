#pragma once

#include "Asset.h"

// @TODO: Out of module include
#include "Material/MaterialCommon.h"
#include "Resource/ResourceCommon.h"

namespace Ion
{
	// Asset Parser Base ------------------------------------------------------------------------

	class ION_API AssetParser : public XMLParser<AssetParser>
	{
	public:
		/**
		 * @brief Construct a new Asset Parser object using an Asset handle
		 * 
		 * @param asset Asset handle to take the path from
		 */
		AssetParser(const Asset& asset);

		/**
		 * @brief Construct a new Asset Parser object using a specified path
		 * 
		 * @param assetPath Asset file path
		 */
		AssetParser(const FilePath& assetPath);

		/**
		 * @brief Start the asset parsing, enter the <IonAsset> node
		 */
		AssetParser& BeginAsset();

		/**
		 * @brief Start the asset parsing, enter the <IonAsset> node
		 * and expect a specific asset type. Fails if the actual type is different
		 * 
		 * @param type Asset type to expect
		 */
		AssetParser& BeginAsset(EAssetType type);

		/**
		 * @brief EnterNode wrapper
		 */
		AssetParser& Begin(const String& nodeName);

		/**
		 * @brief ExitNode wrapper
		 */
		AssetParser& End();

		/**
		 * @brief Parses the <Info> node
		 */
		AssetParser& ParseInfo(EAssetType& outType, GUID& outGuid);

		/**
		 * @brief Parses the <Name> node
		 */
		AssetParser& ParseName(String& outName);

		/**
		 * @brief Fails if the actual type is different than the specified one.
		 */
		AssetParser& ExpectType(EAssetType type);
	};

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
