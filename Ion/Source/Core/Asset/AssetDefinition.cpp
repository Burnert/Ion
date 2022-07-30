#include "IonPCH.h"

#include "AssetDefinition.h"
#include "AssetRegistry.h"
#include "Asset.h"

#include "Core/Task/EngineTaskQueue.h"
#include "Core/File/Collada.h"

#include "Application/EnginePath.h"

#define CHECK_NODE(node, nodeName) IASSET_CHECK_NODE(node, nodeName, m_AssetDefinitionPath)
#define CHECK_ATTR(attr, attrName, nodeName) IASSET_CHECK_ATTR(attr, attrName, nodeName, m_AssetDefinitionPath)

namespace Ion
{
	// AssetDefinition ----------------------------------------------------------------

	AssetDefinition::AssetDefinition(const AssetInitializer& initializer) :
		m_Guid(initializer.Guid),
		m_VirtualPath(initializer.VirtualPath),
		m_AssetDefinitionPath(initializer.AssetDefinitionPath),
		m_AssetReferencePath(initializer.AssetReferencePath),
		m_Type(initializer.Type),
		m_bImportExternal(initializer.bImportExternal),
		m_AssetData(EAssetType::None),
		m_Info({ })
	{
		ParseAssetDefinitionFile(initializer.IAssetXML);
	}

	AssetDefinition::~AssetDefinition()
	{
	}

	bool AssetDefinition::ParseAssetDefinitionFile(const TShared<XMLDocument>& xml)
	{
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset);

		XMLNode* nodeName = nodeIonAsset->first_node(IASSET_NODE_Name);
		m_Info.Name = nodeName ?
			nodeName->value() :
			StringConverter::WStringToString(m_AssetDefinitionPath.LastElement());

		XMLNode* nodeResource = nodeIonAsset->first_node(IASSET_NODE_Resource);
		if (nodeResource)
		{
			XMLNode* nodeCurrentResource = nodeResource->first_node();
			while (nodeCurrentResource)
			{
				m_Info.ResourceUsage.push_back(nodeCurrentResource->name());
				nodeCurrentResource = nodeCurrentResource->next_sibling();
			}
		}

		return true;
	}

	Asset AssetDefinition::GetHandle() const
	{
		return Asset(const_cast<AssetDefinition*>(this));
	}
}
