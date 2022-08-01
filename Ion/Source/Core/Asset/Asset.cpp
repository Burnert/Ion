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

	Result<Asset, IOError, FileNotFoundError> Asset::Resolve(const String& virtualPath)
	{
		AssetDefinition* def = AssetRegistry::Find(virtualPath);
		if (!def)
		{
			//LOG_WARN("Cannot find an asset in a virtual path: \"{0}\"", virtualPath);
			
			FilePath path = ResolveVirtualPath(virtualPath);

			if (!path.Exists())
			{
				LOG_ERROR(L"The file \"{0}\" does not exist.", path.ToString());
				ionthrow(FileNotFoundError, "The file \"{0}\" does not exist.", StringConverter::WStringToString(path.ToString()));
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
				ionthrow(IOError, "The file \"{0}\" could not be parsed.", StringConverter::WStringToString(path.ToString()));
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
		{
			ionbreak("Incorrect base dir was specified.");
			return path;
		}

		String relativeVP = virtualPath.substr(nLast + 1) + ".iasset";
		path += StringConverter::StringToWString(relativeVP);

		return path;
	}

	AssetDefinition* Asset::GetAssetDefinition() const
	{
		ionverify(operator bool(), "Cannot access a null handle.");
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
