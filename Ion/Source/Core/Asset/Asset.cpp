#include "IonPCH.h"

#include "Asset.h"
#include "AssetRegistry.h"
#include "Core/File/XML.h"

#define CHECK_NODE(node, nodeName) IASSET_CHECK_NODE(node, nodeName, m_Path)
#define CHECK_ATTR(attr, attrName, nodeName) IASSET_CHECK_ATTR(attr, attrName, nodeName, m_Path)

namespace Ion
{
	// Asset ----------------------------------------------------------------------

	Asset::Asset() :
		m_Guid(GUID::Zero)
#if ION_DEBUG
		, m_DebugDefinition(nullptr)
#endif
	{
		// Null asset handle (not invalid!)
	}

	Asset::Asset(const GUID& guid) :
		m_Guid(guid)
#if ION_DEBUG
		, m_DebugDefinition(guid.IsInvalid() ? nullptr : AssetRegistry::Find(guid))
#endif
	{
	}

	Asset::~Asset()
	{
	}

	Asset Asset::Find(const GUID& guid)
	{
		AssetDefinition* def = AssetRegistry::Find(guid);
		if (!def)
		{
			return InvalidHandle;
		}
		return def->GetHandle();
	}

	AssetDefinition* Asset::FindAssetDefinition() const
	{
		ionassertnd(m_Guid, "Cannot access a null handle.");
		return AssetRegistry::Find(m_Guid);
	}

	const Asset Asset::InvalidHandle = Asset(GUID::Invalid);

	// AssetFinder ----------------------------------------------------------------

	AssetFinder::AssetFinder(const FilePath& path) :
		m_Path(path)
	{
	}

	Asset AssetFinder::Resolve() const
	{
		if (!Exists())
		{
			LOG_ERROR(L"The file \"{0}\" does not exist.", m_Path.ToString());
			return Asset::InvalidHandle;
		}

		String assetDefinition;
		File::ReadToString(m_Path, assetDefinition);

		TShared<XMLDocument> assetDefinitionDoc = MakeShared<XMLDocument>(assetDefinition);

		AssetInitializer initializer;
		initializer.IAssetXML = assetDefinitionDoc;
		initializer.AssetDefinitionPath = m_Path;

		if (!Parse(assetDefinitionDoc, initializer))
		{
			LOG_ERROR(L"The file \"{0}\" could not be parsed.", m_Path.ToString());
			return Asset::InvalidHandle;
		}

		// The asset might have already been registered.
		if (AssetDefinition* asset = AssetRegistry::Find(initializer.Guid))
		{
			return asset->GetHandle();
		}

		return AssetRegistry::Register(initializer).GetHandle();
	}

	static EAssetType ParseTypeString(const String& sType)
	{
		// @TODO: This should eventually parse the type string as kind of a package
		// This way custom apps would be able to make their own asset types if needed.
		size_t ionPrefix = sType.find("Ion.");
		if (ionPrefix == -1)
			return EAssetType::Invalid;

		StringView svType = StringView(sType).substr(4);
		if (svType == "Image")
			return EAssetType::Image;
		if (svType == "Mesh")
			return EAssetType::Mesh;

		return EAssetType::Invalid;
	}

	bool AssetFinder::Parse(TShared<XMLDocument>& xml, AssetInitializer& outInitializer) const
	{
		// <IonAsset>
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset);

		// <Info>
		XMLNode* nodeInfo = nodeIonAsset->first_node(IASSET_NODE_Info);
		CHECK_NODE(nodeIonAsset, IASSET_NODE_Info);

		// type=
		XMLAttribute* info_attrType = nodeInfo->first_attribute(IASSET_ATTR_Info_type);
		CHECK_ATTR(info_attrType, IASSET_ATTR_Info_type, IASSET_NODE_Info);

		char* csType = info_attrType->value();
		outInitializer.Type = ParseTypeString(csType);
		ionexcept(outInitializer.Type != EAssetType::Invalid, "Invalid asset type.")
			return false;

		// guid=
		XMLAttribute* info_attrGuid = nodeInfo->first_attribute(IASSET_ATTR_guid);
		CHECK_ATTR(info_attrGuid, IASSET_ATTR_guid, IASSET_NODE_Info);

		String sGuid = info_attrGuid->value();
		outInitializer.Guid = GUID(sGuid);
		ionexcept(outInitializer.Guid, "Invalid GUID.")
			return false;

		// <ImportExternal>
		XMLNode* nodeImportExternal = nodeIonAsset->first_node(IASSET_NODE_ImportExternal);
		if (nodeImportExternal)
		{
			outInitializer.bImportExternal = true;

			// path=
			XMLAttribute* import_attrPath = nodeImportExternal->first_attribute(IASSET_ATTR_ImportExternal_path);
			CHECK_ATTR(import_attrPath, IASSET_ATTR_ImportExternal_path, IASSET_NODE_ImportExternal);

			char* csPath = import_attrPath->value();
			FilePath importPath = StringConverter::StringToWString(csPath);
			ionexcept(!importPath.IsEmpty(), "Asset Import External path is empty.")
				return false;

			// If the import path is relative, it means it begins in the directory
			// the .iasset file is in. Append the paths in that case.
			if (importPath.IsRelative())
			{
				FilePath actualPath = m_Path;
				actualPath.Back();
				importPath = Move(actualPath += importPath);
			}

			outInitializer.AssetReferencePath = importPath;
		}

		return true;
	}
}
