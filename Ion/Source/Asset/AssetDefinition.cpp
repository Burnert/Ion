#include "IonPCH.h"

#include "AssetDefinition.h"
#include "AssetRegistry.h"
#include "AssetParser.h"
#include "Asset.h"

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

		auto customParseResult = GetType().Parse(xml);
		fwdthrowall(customParseResult);

		m_CustomData = customParseResult.Unwrap();

		return Ok();
	}
}
