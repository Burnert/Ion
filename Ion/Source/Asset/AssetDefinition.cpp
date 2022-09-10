#include "IonPCH.h"

#include "AssetDefinition.h"
#include "AssetRegistry.h"
#include "AssetParser.h"
#include "Asset.h"

// @TODO: TEMPORARY:
#include "Resource/MeshResource.h"

namespace Ion
{
	// AssetDefinition ----------------------------------------------------------------

	AssetDefinition::AssetDefinition(const AssetInitializer& initializer) :
		m_VirtualPath(initializer.VirtualPath),
		m_AssetDefinitionPath(initializer.AssetDefinitionPath),
		m_Type(nullptr),
		m_Info({ }),
		m_bImportExternal(false)
	{
	}

	AssetDefinition::~AssetDefinition()
	{
	}

	Result<void, IOError> AssetDefinition::ParseAssetDefinitionFile(const std::shared_ptr<XMLDocument>& xml)
	{
		XMLParserResult result = AssetParser(xml)
			.BeginAsset()
			.ParseInfo(m_Type, m_Guid)
			.ParseName(m_Info.Name)
			.TryEnterNode(IASSET_NODE_ImportExternal, [this](AssetParser& parser)
			{
				parser.ParseCurrentAttributes(IASSET_ATTR_path, [&, this](String sPath)
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
						FilePath actualPath = m_AssetDefinitionPath;
						actualPath.Back();
						path = Move(actualPath += path);
					}

					m_AssetImportPath = path;
				});
			})
			.TryEnterNode(IASSET_NODE_Resource, [&](AssetParser& parser)
			{
				parser.EnterEachNode([&](AssetParser& parser)
				{
					m_Info.ResourceUsage.push_back(parser.GetCurrentNodeName());
				});
			})
			.Finalize();

		if (!result.OK())
		{
			result.PrintMessages();
			ionthrow(IOError, result.GetFailMessage());
		}

		XMLArchive ar(EArchiveType::Loading);
		ar.LoadXML(xml);

		fwdthrowall(GetType().Serialize(ar, m_CustomData));

		return Ok();
	}

	Result<void, IOError> AssetDefinition::Serialize(Archive& ar)
	{
		ionassert(ar.IsLoading() || m_Type);

		XMLArchiveAdapter xmlAr = ar;
		xmlAr.SeekRoot();

		xmlAr.EnterNode(IASSET_NODE_IonAsset);

		xmlAr.EnterNode(IASSET_NODE_Info);

		xmlAr.EnterAttribute(IASSET_ATTR_type);
		String sType = ar.IsSaving() ? m_Type->GetName() : EmptyString;
		xmlAr << sType;
		xmlAr.ExitAttribute(); // IASSET_ATTR_type

		if (IAssetType* type = AssetRegistry::FindType(sType))
			m_Type = type;
		else
			ionthrow(IOError, "\"{}\" is an invalid asset type.", sType);

		xmlAr.ExitNode(); // IASSET_NODE_Info

		if (xmlAr.TryEnterNode(IASSET_NODE_Name))
		{
			xmlAr << m_Info.Name;
			xmlAr.ExitNode(); // IASSET_NODE_Name
		}
		else // Can happen only when loading
		{
			m_Info.Name = FilePath(m_VirtualPath).LastElement();
		}

		if (xmlAr.TryEnterNode(IASSET_NODE_ImportExternal))
		{
			m_bImportExternal = true;

			xmlAr.EnterAttribute(IASSET_ATTR_path);
			String sPath = ar.IsSaving() ? m_AssetImportPath.RelativeTo(m_AssetDefinitionPath / "..") : EmptyString;
			xmlAr << sPath;
			xmlAr.ExitAttribute();

			FilePath path = sPath;
			if (path.IsRelative())
			{
				path = m_AssetDefinitionPath / ".." / path;
			}

			m_AssetImportPath = path;

			xmlAr.ExitNode(); // IASSET_NODE_ImportExternal
		}
		else
		{
			m_bImportExternal = false;
		}

		// Serialize the custom data (different for each asset type)
		fwdthrowall(m_Type->Serialize(ar, m_CustomData));

		xmlAr.ExitNode(); // IASSET_NODE_IonAsset
		
		return Ok();
	}
}
