#pragma once

#include "AssetCore.h"

#define ASSET_WORKER_COUNT 4
#define DEFAULT_MESH_POOL_SIZE    (1 << 28) // 256 MB
#define DEFAULT_TEXTURE_POOL_SIZE (1 << 29) // 512 MB

namespace Ion
{
	class AssetManager;

	struct AssetWorkerLoadWork
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

		// Worker Thread Functions ------------------------------------------------

		void WorkerProc();
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
		using WorkerQueue = TQueue<AssetWorkerLoadWork>;
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
		size_t GetAssetAlignment() const;

		//template<> size_t GetAssetAlignment<EAssetType::Mesh>() const;
		//template<> size_t GetAssetAlignment<EAssetType::Texture>() const;

		template<EAssetType Type>
		void DefragmentAssetPool();

		template<EAssetType Type>
		void PrintAssetPool() const;

		template<EAssetType Type>
		TShared<AssetMemoryPoolDebugInfo> GetAssetPoolDebugInfo() const;

		~AssetManager() { }

	protected:
		AssetReference* GetAssetReference(AssetHandle handle);

		void LoadAssetData(AssetReference& ref);
		void UnloadAssetData(AssetReference& ref);

		template<EAssetType Type>
		void* AllocateAssetData(size_t size);

		//template<> void* AllocateAssetData<EAssetType::Mesh>(size_t size);
		//template<> void* AllocateAssetData<EAssetType::Texture>(size_t size);

	private:
		void ScheduleAssetLoadWork(AssetReference& ref);

		template<typename T>
		void AddMessage(T& message);

		template<typename T>
		static void _DispatchMessage(T& message); // underscore because there's a Windows macro called DispatchMessage

		void OnAssetLoaded(OnAssetLoadedMessage& message);
		void OnAssetUnloaded(OnAssetUnloadedMessage& message);
		void OnAssetRealloc(OnAssetReallocMessage& message);

	private:
		AssetMemoryPool m_MeshAssetPool;
		AssetMemoryPool m_TextureAssetPool;

		THashMap<AssetID, AssetReference> m_Assets;
		THashMap<void*, AssetReference*> m_LoadedAssets;

		MessageQueue m_MessageQueue;
		Mutex m_MessageQueueMutex;

		TArray<AssetWorker> m_AssetWorkers;

		WorkerQueue m_WorkQueue;
		Mutex m_WorkQueueMutex;
		ConditionVariable m_WorkQueueCV;

		AssetManager();

		friend class AssetWorker;
		friend class AssetInterface;
		friend class Application;

		template<EAssetType Type>
		friend struct TAssetPoolFromType;
	};

	template<EAssetType Type>
	struct TAssetPoolFromType;
	template<>
	struct TAssetPoolFromType<EAssetType::Mesh>    { static constexpr AssetMemoryPool AssetManager::* Ref = &AssetManager::m_MeshAssetPool; };
	template<>
	struct TAssetPoolFromType<EAssetType::Texture> { static constexpr AssetMemoryPool AssetManager::* Ref = &AssetManager::m_TextureAssetPool; };
	#define _GetAssetPoolFromType(arg) this->*TAssetPoolFromType<arg>::Ref
}
#include "AssetManager.inl"
