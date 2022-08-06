#include "IonPCH.h"

#include "Asset.h"
#include "AssetRegistry.h"
#include "AssetParser.h"
#include "AssetDefinition.h"
#include "Core/File/XML.h"
#include "Core/File/Collada.h"
#include "Application/EnginePath.h"

namespace Ion
{
	// Asset ----------------------------------------------------------------------

	const Asset Asset::None = Asset();

	Asset::Asset() :
		m_AssetPtr(nullptr)
	{
	}

	Asset::Asset(AssetDefinition* asset) :
		m_AssetPtr(asset)
	{
		ionassert(AssetRegistry::IsValid(asset));
	}

	Asset::~Asset()
	{
	}

	Result<Asset, IOError, FileNotFoundError> Asset::Resolve(const String& virtualPath)
	{
		AssetDefinition* def = AssetRegistry::Find(virtualPath);
		if (!def)
		{
			FilePath path = ResolveVirtualPath(virtualPath);
			return RegisterAsset(path, virtualPath);
		}
		return def->GetHandle();
	}

	Result<Asset, IOError, FileNotFoundError> Asset::RegisterExternal(const FilePath& path, const String& customVirtualPath)
	{
		ionassert(!path.IsEmpty());
		ionassert(IsValidVirtualPath(customVirtualPath), "Invalid virtual path. -> {}", customVirtualPath);
		ionassert(!IsStandardVirtualRoot(GetRootOfVirtualPath(customVirtualPath)), "Don't use a standard root in a custom virtual path.");

		return RegisterAsset(path, customVirtualPath);
	}

	FilePath Asset::ResolveVirtualPath(const String& virtualPath)
	{
		ionassert(IsValidVirtualPath(virtualPath), "Invalid virtual path. -> {}", virtualPath);

		String root = GetRootOfVirtualPath(virtualPath);
		ionassert(AssetRegistry::IsVirtualRootRegistered(root), "Virtual root {} has not been registered.", root);

		FilePath path = AssetRegistry::ResolveVirtualRoot(root);

		String rest = GetRestOfVirtualPath(virtualPath) + Asset::FileExtension;
		ionassert(rest != Asset::FileExtension);
		path += rest;

		return path;
	}

	AssetDefinition* Asset::GetAssetDefinition() const
	{
		ionverify(IsValid(), "Cannot access a null handle.");
		return AssetRegistry::IsValid(m_AssetPtr) ? m_AssetPtr : nullptr;
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

					FilePath path = sPath;

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

	Result<Asset, IOError, FileNotFoundError> Asset::RegisterAsset(const FilePath& path, const String& virtualPath)
	{
		if (!path.Exists())
		{
			LOG_ERROR("The file \"{0}\" does not exist.", path.ToString());
			ionthrow(FileNotFoundError, "The file \"{0}\" does not exist.", path.ToString());
		}

		String assetDefinition;
		ionmatchresult(File::ReadToString(path),
			mfwdthrowall
			melse assetDefinition = R.Unwrap();
		);

		AssetInitializer initializer;
		initializer.IAssetXML = MakeShared<XMLDocument>(assetDefinition);
		initializer.AssetDefinitionPath = path;
		initializer.VirtualPath = virtualPath;

		if (!Parse(/*in out*/ initializer))
		{
			LOG_ERROR("The file \"{0}\" could not be parsed.", path.ToString());
			ionthrow(IOError, "The file \"{0}\" could not be parsed.", path.ToString());
		}

		return AssetRegistry::Register(initializer).GetHandle();
	}

	bool Asset::IsVirtualRoot(const StringView& root)
	{
		return root[0] == '[' && root[root.length() - 1] == ']';
	}

	bool Asset::IsStandardVirtualRoot(const StringView& root)
	{
		return
			root == VirtualRoot::Engine  ||
			root == VirtualRoot::Shaders ||
			root == VirtualRoot::Game;
	}

	bool Asset::IsValidVirtualPath(const String& virtualPath)
	{
		if (virtualPath.find("../") != String::npos ||
			virtualPath.find("/..") != String::npos)
			return false;

		size_t iSlash = virtualPath.find_first_of('/');
		if (iSlash == String::npos)
			return IsVirtualRoot(virtualPath);

		StringView root = StringView(virtualPath).substr(0, iSlash);
		if (!IsVirtualRoot(root))
			return false;
		// @TODO: Check the rest
		return true;
	}

	String Asset::GetRootOfVirtualPath(const String& virtualPath)
	{
		ionassert(IsValidVirtualPath(virtualPath));

		size_t iSlash = virtualPath.find_first_of('/');
		if (iSlash == String::npos && IsVirtualRoot(virtualPath))
			return virtualPath;
		StringView root = StringView(virtualPath).substr(0, iSlash);
		if (IsVirtualRoot(root))
			return String(root);
		return "";
	}

	String Asset::GetRestOfVirtualPath(const String& virtualPath)
	{
		ionassert(IsValidVirtualPath(virtualPath));

		size_t iSlash = virtualPath.find_first_of('/');
		if (iSlash == String::npos)
			return "";
		return virtualPath.substr(iSlash);
	}

	EAssetType ParseAssetTypeString(const String& sType)
	{
		// @TODO: This should eventually parse the type string as kind of a package
		// This way custom apps would be able to make their own asset types if needed.
		size_t ionPrefix = sType.find("Ion.");
		if (ionPrefix == -1)
			return EAssetType::Invalid;

		return TEnumParser<EAssetType>::FromString(sType.substr(4)).value_or(EAssetType::Invalid);
	}

	void AssetImporter::ImportColladaMeshAsset(const TShared<AssetFileMemoryBlock>& block, MeshAssetData& outData)
	{
		String collada((const char*)block->Ptr, block->Count);

		// @TODO: Refactor the ColladaDocument class a bit
		TUnique<ColladaDocument> colladaDoc = MakeUnique<ColladaDocument>(collada);
		ColladaData colladaData = colladaDoc->Parse().Unwrap();

		outData.Layout = colladaData.Layout;

		outData.Vertices.Ptr = new float[colladaData.VertexAttributeCount];
		outData.Vertices.Count = colladaData.VertexAttributeCount;

		outData.Indices.Ptr = new uint32[colladaData.IndexCount];
		outData.Indices.Count = colladaData.IndexCount;

		memcpy(outData.Vertices.Ptr, colladaData.VertexAttributes, colladaData.VertexAttributeCount * sizeof(float));
		memcpy(outData.Indices.Ptr, colladaData.Indices, colladaData.IndexCount * sizeof(uint32));
	}

	void AssetImporter::ImportImageAsset(const TShared<AssetFileMemoryBlock>& block, Image& outImage)
	{
		outImage.Load(block->Ptr, block->Count);
	}
}
