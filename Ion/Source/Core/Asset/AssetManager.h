#pragma once

#include "AssetCore.h"
#include "AssetMemory.h"

#define ASSET_WORKER_COUNT 2
#define DEFAULT_TEXTURE_POOL_SIZE (1 << 29) // 512 MB

namespace Ion
{
	class AssetManager;

	struct AssetWorkerWork
	{
		AssetReference* RefPtr;
		TFunction<void(AssetReference&)> OnLoad;
	};

	class ION_API AssetWorker
	{
	public:
		AssetWorker(AssetManager* owner);

	private:
		void Start();

		void WorkerProc();

		void LoadAsset(EAssetType type, FilePath location, AssetReference* refPtr);

	private:
		Thread m_WorkerThread;
		AssetManager* m_Owner;

		friend class AssetManager;
	};

	class ION_API AssetManager
	{
	public:
		using WorkerQueue = TQueue<AssetWorkerWork>;

		static AssetManager* Get()
		{
			static AssetManager* c_Instance = new AssetManager;
			return c_Instance;
		}

		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		void Init();
		void Shutdown();

		AssetReference& CreateAsset(EAssetType type, FilePath location);
		void DeleteAsset(AssetHandle handle);

		uint64 GetAssetSize(AssetHandle handle);

		AssetData GetAssetDataImmediate(AssetHandle handle);

		template<typename Lambda>
		void GetAssetData(AssetHandle handle, Lambda callback)
		{

		}

		AssetReference* GetAssetReference(AssetHandle handle);

		inline bool IsAssetLoaded(AssetHandle handle) const
		{
			return false;
		}

		inline bool IsHandleValid(AssetHandle handle) const
		{
			return false;
		}

		~AssetManager() { }

	protected:
		template<EAssetType Type>
		void* AllocateAssetData(size_t size) { }

		template<> void* AllocateAssetData<EAssetType::Texture>(size_t size);

	private:
		void ScheduleAssetLoadWork(AssetReference& ref);

	private:
		AssetMemoryPool m_TextureAssetPool;

		THashMap<AssetID, AssetReference> m_Assets;

		TArray<AssetWorker> m_WorkerThreads;
		WorkerQueue m_WorkQueue;
		Mutex m_QueueMutex;
		ConditionVariable m_QueueCV;

		AssetManager();

		friend class AssetWorker;
		friend class Application;
		//friend TFunction<void(AssetReference&)>;
	};
}
#include "AssetManager.inl"
