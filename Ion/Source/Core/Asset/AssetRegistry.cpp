#include "IonPCH.h"

#include "AssetRegistry.h"
#include "Asset.h"
#include "Core/Task/EngineTaskQueue.h"

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

	// FAssetLoadWork -----------------------------------------------

	FAssetLoadWork::FAssetLoadWork(const FilePath& assetPath) :
		m_AssetPath(assetPath)
	{
	}

	void FAssetLoadWork::Schedule()
	{
		Execute = [Copy = *this](IMessageQueueProvider& queue)
		{
			ionassert(Copy.OnLoad);
			ionassert(Copy.OnError);

			// @TODO: Fix, temporary memory leak
			char* path = new char[200];
			strcpy_s(path, 200, StringConverter::WStringToString(Copy.m_AssetPath.ToString()).c_str());

			// @TODO: Load
			AssetData data;
			data.Data = path;
			data.Size = 200;

			std::this_thread::sleep_for(std::chrono::seconds(2));

			queue.PushMessage(FTaskMessage([OnLoad = Copy.OnLoad, data]
			{
				OnLoad(data);
			}));
		};

		AssetRegistry::ScheduleWork(*this);
	}

	// AssetRegistry ----------------------------------------------------------------

	Asset AssetDefinition::GetHandle() const
	{
		return Asset(m_Guid);
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

	AssetRegistry::AssetRegistry() :
		m_WorkQueue(EngineTaskQueue::Get())
	{
	}

	AssetRegistry& AssetRegistry::Get()
	{
		static AssetRegistry* c_Instance = new AssetRegistry;
		return *c_Instance;
	}
}
