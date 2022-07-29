#include "IonPCH.h"

#include "Asset.h"
#include "AssetRegistry.h"
#include "Core/File/XML.h"
#include "Application/EnginePath.h"

#define CHECK_NODE(node, nodeName) IASSET_CHECK_NODE(node, nodeName, m_Path)
#define CHECK_ATTR(attr, attrName, nodeName) IASSET_CHECK_ATTR(attr, attrName, nodeName, m_Path)

namespace Ion
{
	// Asset ----------------------------------------------------------------------

	Asset::Asset() :
		m_AssetPtr(nullptr)
	{
		// Null asset handle (not invalid!)
		m_AssetPtr.SetMetaFlag<0>(true);
	}

	Asset::Asset(AssetDefinition* asset) :
		m_AssetPtr(asset)
	{
		ionassert(AssetRegistry::IsRegistered(asset));
	}

	Asset::Asset(InvalidInitializerT) :
		m_AssetPtr(nullptr)
	{
	}

	Asset::~Asset()
	{
	}

	Asset Asset::Find(const String& virtualPath)
	{
		AssetDefinition* def = AssetRegistry::Find(virtualPath);
		if (!def)
		{
			LOG_WARN("Cannot find an asset in a virtual path: \"{0}\"", virtualPath);
			return InvalidHandle;
		}
		return def->GetHandle();
	}

	FilePath Asset::ResolveVirtualPath(const String& virtualPath)
	{
		size_t nFirst = virtualPath.find_first_of('[', 0);
		ionassert(nFirst == 0, "Invalid virtual path.");
		size_t nLast = virtualPath.find_first_of(']', nFirst);
		ionassert(nLast != String::npos, "Invalid virtual path.");
		ionassert(nLast + 1 < virtualPath.size(), "Invalid virtual path.");

		FilePath path;

		StringView baseDirId = StringView(virtualPath).substr(nFirst, nLast + 1 - nFirst);
		if (baseDirId == "[Engine]")
			path = EnginePath::GetEngineContentPath();
		else if (baseDirId == "[Shaders]")
			path = EnginePath::GetShadersPath();
		// @TODO: [Project] / [Game] path
		else
			return path;

		String relativeVP = virtualPath.substr(nLast + 1) + ".iasset";
		path += StringConverter::StringToWString(relativeVP);

		return path;
	}

	AssetDefinition* Asset::GetAssetDefinition() const
	{
		ionassertnd(operator bool(), "Cannot access a null handle.");
		return AssetRegistry::IsRegistered(*this) ? m_AssetPtr.Get() : nullptr;
	}

	const Asset Asset::InvalidHandle = Asset::InvalidInitializerT();
	const Asset Asset::None = Asset();

	// AssetFinder ----------------------------------------------------------------

	AssetFinder::AssetFinder(const FilePath& path) :
		m_Path(path)
	{
	}

	AssetFinder::AssetFinder(const String& virtualPath) :
		m_Path(Asset::ResolveVirtualPath(virtualPath)),
		m_VirtualPath(virtualPath)
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

		AssetInitializer initializer;
		initializer.IAssetXML = MakeShared<XMLDocument>(assetDefinition);
		initializer.AssetDefinitionPath = m_Path;
		initializer.VirtualPath = m_VirtualPath;

		if (!Parse(initializer.IAssetXML, initializer))
		{
			LOG_ERROR(L"The file \"{0}\" could not be parsed.", m_Path.ToString());
			return Asset::InvalidHandle;
		}

		// The asset might have already been registered.
		if (AssetDefinition* asset = AssetRegistry::Find(initializer.VirtualPath))
		{
			return asset->GetHandle();
		}

		return AssetRegistry::Register(initializer).GetHandle();
	}

	EAssetType ParseAssetTypeString(const String& sType)
	{
		// @TODO: This should eventually parse the type string as kind of a package
		// This way custom apps would be able to make their own asset types if needed.
		size_t ionPrefix = sType.find("Ion.");
		if (ionPrefix == -1)
			return EAssetType::Invalid;

		StringView svType = StringView(sType).substr(4);
		if (svType == "Generic")
			return EAssetType::Generic;
		if (svType == "Image")
			return EAssetType::Image;
		if (svType == "Mesh")
			return EAssetType::Mesh;
		if (svType == "Data")
			return EAssetType::Data;
		if (svType == "Material")
			return EAssetType::Material;
		if (svType == "MaterialInstance")
			return EAssetType::MaterialInstance;

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
		XMLAttribute* info_attrType = nodeInfo->first_attribute(IASSET_ATTR_type);
		CHECK_ATTR(info_attrType, IASSET_ATTR_type, IASSET_NODE_Info);

		char* csType = info_attrType->value();
		outInitializer.Type = ParseAssetTypeString(csType);
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
			XMLAttribute* import_attrPath = nodeImportExternal->first_attribute(IASSET_ATTR_path);
			CHECK_ATTR(import_attrPath, IASSET_ATTR_path, IASSET_NODE_ImportExternal);

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
