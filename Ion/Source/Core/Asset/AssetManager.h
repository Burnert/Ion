#pragma once

#include "AssetCore.h"
#include "AssetMemory.h"

#define ASSET_WORKER_COUNT 4
#define DEFAULT_MESH_POOL_SIZE (1 << 28)    // 256 MB
#define DEFAULT_TEXTURE_POOL_SIZE (1 << 29) // 512 MB

namespace Ion
{
	class AssetManager;

	struct AssetWorkerWork
	{
		AssetReference* RefPtr;
		TFunction<void(AssetReference&)> OnLoad;
		TFunction<void(AssetReference&)> OnError;
	};

	class ION_API AssetWorker
	{
	public:
		AssetWorker();
		AssetWorker(AssetManager* owner);

	private:
		void Start();
		// Lock the WorkQueue before calling
		void Exit();

		void WorkerProc();

		// Worker Thread Functions ------------------------------------------------

		bool LoadAsset(AssetReference* refPtr);

	private:
		Thread m_WorkerThread;
		AssetManager* m_Owner;
		bool m_bExit;

		friend class AssetManager;
	};

	class ION_API AssetManager
	{
	public:
		using WorkerQueue = TQueue<AssetWorkerWork>;
		using MessageQueue = TQueue<AssetMessageBuffer>;

		inline static AssetManager* Get()
		{
			static AssetManager* c_Instance = new AssetManager;
			return c_Instance;
		}

		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		void Init();
		void Update();
		void Shutdown();

		template<EAssetType Type>
		AssetHandle CreateAsset(FilePath location);
		AssetHandle CreateAsset(EAssetType type, FilePath location);
		void DeleteAsset(AssetHandle handle);

		const AssetReference* GetAssetReference(AssetHandle handle) const;

		inline bool IsAssetLoaded(AssetHandle handle) const
		{
			return (bool)GetAssetReference(handle)->Data.Ptr;
		}

		inline bool IsHandleValid(AssetHandle handle) const
		{
			return handle.m_ID != INVALID_ASSET_ID &&
				m_Assets.find(handle.m_ID) != m_Assets.end();
		}

		template<typename ForEach>
		void IterateMessages(ForEach forEach);

		template<EAssetType Type>
		size_t GetAssetAlignment() const { }

		template<> size_t GetAssetAlignment<EAssetType::Mesh>() const;
		template<> size_t GetAssetAlignment<EAssetType::Texture>() const;

		~AssetManager() { }

	protected:
		AssetReference* GetAssetReference(AssetHandle handle);
		void LoadAssetData(AssetReference& ref);

		template<EAssetType Type>
		void* AllocateAssetData(size_t size) { }

		template<> void* AllocateAssetData<EAssetType::Mesh>(size_t size);
		template<> void* AllocateAssetData<EAssetType::Texture>(size_t size);

	private:
		void ScheduleAssetLoadWork(AssetReference& ref);
		void UnloadAsset(AssetReference& ref);

		template<typename T>
		void AddMessage(T& message);

		template<typename T>
		static void _DispatchMessage(T& message); // underscore because there's a Windows macro called DispatchMessage

	private:
		AssetMemoryPool m_MeshAssetPool;
		AssetMemoryPool m_TextureAssetPool;

		THashMap<AssetID, AssetReference> m_Assets;

		MessageQueue m_MessageQueue;
		Mutex m_MessageQueueMutex;

		TFixedArray<AssetWorker, ASSET_WORKER_COUNT> m_WorkerThreads;

		WorkerQueue m_WorkQueue;
		Mutex m_WorkQueueMutex;
		ConditionVariable m_WorkQueueCV;

		AssetManager();

		friend class AssetWorker;
		friend class AssetInterface;
		friend class Application;
	};
}
#include "AssetManager.inl"
