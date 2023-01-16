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

	void AssetDefinition::Refresh()
	{
		ionassert(!m_AssetDefinitionPath.IsEmpty());
		ionassert(!m_VirtualPath.empty());
		ionassert(m_CustomData);
		ionassert(m_Type);

		YAMLArchive ar(EArchiveType::Loading);
		File file(m_AssetDefinitionPath);
		ar.LoadFromFile(file);

		m_Type->Serialize(ar, m_CustomData)
			.Err([this](Error& err) { AssetLogger.Error("Cannot refresh asset \"{}\".\n{}", m_VirtualPath, err.Message); });
	}

	void AssetDefinition::SaveToDisk()
	{
		ionassert(!m_AssetDefinitionPath.IsEmpty());
		ionassert(!m_VirtualPath.empty());
		ionassert(m_CustomData);
		ionassert(m_Type);

		YAMLArchive ar(EArchiveType::Saving);

		if (!Serialize(ar).Err([this](Error& err) { AssetLogger.Error("Cannot save asset \"{}\" to file.\n{}", m_VirtualPath, err.Message); }))
			return;

		File file(m_AssetDefinitionPath);
		ar.SaveToFile(file);
	}

	Result<void, IOError> AssetDefinition::Serialize(Archive& ar)
	{
		ionassert(ar.IsLoading() || m_Type);

		ArchiveNode nodeRoot = ar.EnterRootNode();

		ArchiveNode nodeType = ar.EnterNode(nodeRoot, "Type", EArchiveNodeType::Value);

		String sType = ar.IsSaving() ? m_Type->GetName() : EmptyString;
		nodeType &= sType;

		if (IAssetType* type = AssetRegistry::FindType(sType))
			m_Type = type;
		else
			ionthrow(IOError, "\"{}\" is an invalid asset type.", sType);

		ArchiveNode nodeName = ar.EnterNode(nodeRoot, "Name", EArchiveNodeType::Value);
		nodeName &= m_Info.Name;

		if (ar.IsLoading() && m_Info.Name.empty())
		{
			m_Info.Name = FilePath(m_VirtualPath).LastElement();
		}

		if (ArchiveNode nodeImportExternal = ar.EnterNode(nodeRoot, "ImportExternal", EArchiveNodeType::Value))
		{
			m_bImportExternal = true;

			String sPath = ar.IsSaving() ? m_AssetImportPath.RelativeTo(m_AssetDefinitionPath / "..") : EmptyString;
			nodeImportExternal &= sPath;

			FilePath path = sPath;
			if (path.IsRelative())
			{
				path = m_AssetDefinitionPath / ".." / path;
			}

			m_AssetImportPath = path;
		}
		else
		{
			m_bImportExternal = false;
		}

		// Serialize the custom data (different for each asset type)
		fwdthrowall(m_Type->Serialize(ar, m_CustomData));

		return Ok();
	}
}
