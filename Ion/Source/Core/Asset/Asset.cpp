#include "IonPCH.h"

#include "Asset.h"
#include "AssetRegistry.h"
#include "AssetParser.h"
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

	bool Asset::Parse(AssetInitializer& inOutInitializer)
	{
		return AssetParser(inOutInitializer.AssetDefinitionPath)
			.BeginAsset()
			.Begin(IASSET_NODE_Info) // <Info>
			.ParseCurrentAttributes(IASSET_ATTR_type, [&inOutInitializer](String type)
			{
				inOutInitializer.Type = ParseAssetTypeString(type);
			})
			.FailIf([&inOutInitializer]
			{
				return inOutInitializer.Type == EAssetType::Invalid;
			}, "Invalid asset type.")
			.ParseCurrentAttributeTyped(IASSET_ATTR_guid, inOutInitializer.Guid)
			.End() // </Info>
			.TryEnterNode(IASSET_NODE_ImportExternal, [&inOutInitializer](AssetParser& parser)
			{
				parser.ParseCurrentAttributes(IASSET_ATTR_path, [&parser, &inOutInitializer](String sPath)
				{
					if (sPath.empty())
					{
						parser.Fail("Asset Import External path is empty.");
						return;
					}

					FilePath path = StringConverter::StringToWString(sPath);

					// If the import path is relative, it means it begins in the directory
					// the .iasset file is in. Append the paths in that case.
					if (path.IsRelative())
					{
						FilePath actualPath = inOutInitializer.AssetDefinitionPath;
						actualPath.Back();
						path = Move(actualPath += path);
					}

					inOutInitializer.AssetReferencePath = path;
				});
			})
			.Finalize()
			.OK();
	}

	Asset::~Asset()
	{
	}

	Asset Asset::Resolve(const String& virtualPath)
	{
		AssetDefinition* def = AssetRegistry::Find(virtualPath);
		if (!def)
		{
			//LOG_WARN("Cannot find an asset in a virtual path: \"{0}\"", virtualPath);
			
			FilePath path = ResolveVirtualPath(virtualPath);

			if (!path.Exists())
			{
				LOG_ERROR(L"The file \"{0}\" does not exist.", path.ToString());
				return InvalidHandle;
			}

			String assetDefinition;
			File::ReadToString(path, assetDefinition);

			AssetInitializer initializer;
			initializer.IAssetXML = MakeShared<XMLDocument>(assetDefinition);
			initializer.AssetDefinitionPath = path;
			initializer.VirtualPath = virtualPath;

			if (!Parse(/*in out*/ initializer))
			{
				LOG_ERROR(L"The file \"{0}\" could not be parsed.", path.ToString());
				return Asset::InvalidHandle;
			}

			return AssetRegistry::Register(initializer).GetHandle();
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
}
