#include "IonPCH.h"

#include "AssetDefinition.h"
#include "AssetRegistry.h"
#include "AssetParser.h"
#include "Asset.h"

#include "Core/Task/EngineTaskQueue.h"
#include "Core/File/Collada.h"

#include "Application/EnginePath.h"

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
		m_Info({ })
	{
		ParseAssetDefinitionFile(initializer.AssetDefinitionPath);
	}

	AssetDefinition::~AssetDefinition()
	{
	}

	bool AssetDefinition::ParseAssetDefinitionFile(const FilePath& path)
	{
		return AssetParser(path)
			.BeginAsset()
			.ParseName(m_Info.Name)
			.TryEnterNode(IASSET_NODE_Resource, [&](AssetParser& parser)
			{
				parser.EnterEachNode([&](AssetParser& parser)
				{
					m_Info.ResourceUsage.push_back(parser.GetCurrentNodeName());
				});
			})
			.Finalize()
			.OK();
	}
}
