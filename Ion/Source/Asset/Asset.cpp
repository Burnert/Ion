#include "IonPCH.h"

#include "Asset.h"
#include "AssetRegistry.h"
#include "AssetParser.h"
#include "AssetDefinition.h"
#include "Collada.h"

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
		if (AssetDefinition* def = AssetRegistry::Find(virtualPath))
			return def->GetHandle();

		FilePath path = ResolveVirtualPath(virtualPath);
		return RegisterAsset(path, virtualPath);
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

	Result<Asset, IOError, FileNotFoundError> Asset::RegisterAsset(const FilePath& path, const String& virtualPath)
	{
		if (!path.Exists())
		{
			AssetLogger.Error("The file \"{0}\" does not exist.", path.ToString());
			ionthrow(FileNotFoundError, "The file \"{0}\" does not exist.", path.ToString());
		}

		String assetDefinition;
		ionmatchresult(File::ReadToString(path),
			mfwdthrowall
			melse assetDefinition = R.Unwrap();
		);

		AssetInitializer initializer(std::make_shared<XMLDocument>(assetDefinition), virtualPath, path);
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

	std::shared_ptr<ImportedMeshData> AssetImporter::ImportColladaMeshAsset(const std::shared_ptr<AssetFileMemoryBlock>& block)
	{
		std::shared_ptr<ImportedMeshData> meshData = std::make_shared<ImportedMeshData>();

		String collada((const char*)block->Ptr, block->Count);

		// @TODO: Refactor the ColladaDocument class a bit
		std::unique_ptr<ColladaDocument> colladaDoc = std::make_unique<ColladaDocument>(collada);
		// @TODO: Handle errors
		ColladaData colladaData = colladaDoc->Parse().Unwrap();

		meshData->Layout = colladaData.Layout;

		meshData->Vertices.Ptr = new float[colladaData.VertexAttributeCount];
		meshData->Vertices.Count = colladaData.VertexAttributeCount;

		meshData->Indices.Ptr = new uint32[colladaData.IndexCount];
		meshData->Indices.Count = colladaData.IndexCount;

		memcpy(meshData->Vertices.Ptr, colladaData.VertexAttributes, colladaData.VertexAttributeCount * sizeof(float));
		memcpy(meshData->Indices.Ptr, colladaData.Indices, colladaData.IndexCount * sizeof(uint32));

		return meshData;
	}

	std::shared_ptr<Image> AssetImporter::ImportImageAsset(const std::shared_ptr<AssetFileMemoryBlock>& block)
	{
		std::shared_ptr<Image> image = std::make_shared<Image>();
		image->Load(block->Ptr, block->Count);
		return image;
	}
}
