#include "IonPCH.h"

#include "AssetRegistry.h"
#include "Asset.h"

namespace Ion
{
	// AssetDefinition ----------------------------------------------------------------

	AssetDefinition::AssetDefinition(const AssetInitializer& initializer) :
		m_Guid(initializer.Guid),
		m_AssetDefinitionPath(initializer.AssetDefinitionPath),
		m_AssetReferencePath(initializer.AssetReferencePath),
		m_Type(initializer.Type),
		m_bImportExternal(initializer.bImportExternal),
		m_bIsLoaded(false)
	{
		ParseAssetDefinitionFile();
	}

	AssetDefinition::~AssetDefinition()
	{
	}

	void AssetDefinition::ParseAssetDefinitionFile()
	{
		// @TODO: Parse the rest of the file here
	}

	// AssetRegistry ----------------------------------------------------------------

	Asset AssetDefinition::GetHandle() const
	{
		return Asset(m_Guid);
	}

	void AssetRegistry::Update()
	{
		// Dispatch all the messages at the beginning of each frame.
		Get().m_WorkQueue.DispatchMessages();
	}

	AssetDefinition& AssetRegistry::Register(const AssetInitializer& initializer)
	{
		auto it = Get().m_Assets.find(initializer.Guid);
		if (it != Get().m_Assets.end())
		{
			LOG_ERROR("Cannot register the asset. An asset with the same GUID {{{0}}} already exists.", initializer.Guid);
			return it->second;
		}

		auto& [guid, assetDef] = *Get().m_Assets.emplace(initializer.Guid, AssetDefinition(initializer)).first;
		return assetDef;
	}

	AssetDefinition* AssetRegistry::Find(const GUID& guid)
	{
		auto it = Get().m_Assets.find(guid);
		if (it == Get().m_Assets.end())
		{
			LOG_WARN("Cannot find asset with GUID {{{0}}}.", guid.ToString());
			return nullptr;
		}

		return &it->second;
	}

	AssetRegistry::AssetRegistry()
	{
	}

	AssetRegistry& AssetRegistry::Get()
	{
		static AssetRegistry* c_Instance = new AssetRegistry;
		return *c_Instance;
	}
}
