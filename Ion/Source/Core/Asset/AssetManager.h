#pragma once

#include "AssetCore.h"

#define ASSET_WORKER_COUNT 4
#define DEFAULT_MESH_POOL_SIZE    (1 << 5)//(1 << 28) // 256 MB
#define DEFAULT_TEXTURE_POOL_SIZE (1 << 5)//(1 << 29) // 512 MB

namespace Ion
{
	class AssetManager;

	enum class EAssetWorkerWorkType : uint8
	{
		Null = 0,
		Load,
		Realloc,
	};

#define ASSET_WORK __declspec(novtable)

	struct ASSET_WORK AssetWorkerWorkBase
	{
		EAssetWorkerWorkType GetType() const
		{
			// The first field of the Work object must be an EAssetWorkerWorkType
			return *(EAssetWorkerWorkType*)this;
		}
	};

	using AssetWork_OnLoadEvent       = TFunction<void(AssetReference&)>;
	using AssetWork_OnLoadErrorEvent  = TFunction<void(AssetReference&)>;
	using AssetWork_OnAllocErrorEvent = TFunction<void(AssetReference&, AllocError_Details&)>;

	struct ASSET_WORK AssetWorkerLoadWork : public AssetWorkerWorkBase
	{
		EAssetWorkerWorkType Type = EAssetWorkerWorkType::Load;

		AssetWork_OnLoadEvent OnLoad;
		AssetWork_OnLoadErrorEvent OnLoadError;
		AssetWork_OnAllocErrorEvent OnAllocError;

		AssetReference* RefPtr;
	};

	using AssetWork_OnReallocEvent = TFunction<void(EAssetType)>;

	struct ASSET_WORK AssetWorkerReallocWork : public AssetWorkerWorkBase
	{
		EAssetWorkerWorkType Type = EAssetWorkerWorkType::Realloc;

		AssetWork_OnReallocEvent OnRealloc;

		AssetMemoryPool* PoolPtr;
		EAssetType AssetPoolType;
	};

	class ION_API AssetWorker
	{
	public:
		AssetWorker();

	private:
		void Start();
		void Exit();

		// Worker Thread Functions ------------------------------------------------

		void WorkerProc();
		void DispatchWork(const TShared<AssetWorkerWorkBase>& work);
		void DoLoadWork(const TShared<AssetWorkerLoadWork>& work);
		void DoReallocWork(const TShared<AssetWorkerReallocWork>& work);

		// Load Work related

		bool LoadMesh(File& file, const TShared<AssetWorkerLoadWork>& work, void** outDataPtr, size_t* outDataSize);
		bool LoadTexture(File& file, const TShared<AssetWorkerLoadWork>& work, void** outDataPtr, size_t* outDataSize);
		template<EAssetType Type>
		void* AllocateAssetData(size_t size, AllocError_Details& outErrorData);

	private:
		Thread m_WorkerThread;
		bool m_bExit;
		EAssetWorkerWorkType m_CurrentWork;

		friend class AssetManager;
	};

	class ION_API AssetManager
	{
	public:
		using WorkQueue = TQueue<TShared<AssetWorkerWorkBase>>;
		using MessageQueue = TPriorityQueue<AssetMessageBuffer, TArray<AssetMessageBuffer>, std::greater<AssetMessageBuffer>>;

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
			return GetAssetReference(handle)->IsLoaded();
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

		template<EAssetType Type>
		void DefragmentAssetPool();

		template<EAssetType Type>
		AssetMemoryPool& GetAssetPool();

		template<EAssetType Type>
		const AssetMemoryPool& GetAssetPool() const;

		template<EAssetType Type>
		void PrintAssetPool() const;

		template<EAssetType Type>
		TShared<AssetMemoryPoolDebugInfo> GetAssetPoolDebugInfo() const;

		~AssetManager() { }

	private:
		AssetReference* GetAssetReference(AssetHandle handle);

		void LoadAssetData(AssetReference& ref);
		void UnloadAssetData(AssetReference& ref);

		AssetMemoryPool* GetAssetMemoryPool(EAssetType type);

	private:
		void HandleAssetRealloc(void* oldLocation, void* newLocation);

		// Worker related

		template<typename T>
		void ScheduleAssetWorkerWork(T& work);
		void ScheduleLoadWork(AssetReference& ref);

		// Not thread-safe, lock m_WorkQueueMutex before use
		bool IsAnyWorkerWorking() const;
		void WaitForWorkers() const;

		// End of Worker related

		template<typename T>
		void AddMessage(T& message);

		template<typename T>
		static void _DispatchMessage(T& message); // underscore because there's a Windows macro called DispatchMessage

		void DispatchMessages();

		void OnAssetLoaded(OnAssetLoadedMessage& message);
		void OnAssetAllocError(OnAssetAllocErrorMessage& message);
		void OnAssetUnloaded(OnAssetUnloadedMessage& message);
		void OnAssetRealloc(OnAssetReallocMessage& message);

		void ResolveErrors();
		void ResolveAllocError(AssetReference& ref, AllocError_Details& details);

	private:
		AssetMemoryPool m_MeshAssetPool;
		AssetMemoryPool m_TextureAssetPool;

		THashMap<AssetID, AssetReference> m_Assets;
		THashMap<void*, AssetReference*> m_LoadedAssets;
		TArray<AssetReference*> m_ErrorAssets;

		MessageQueue m_MessageQueue;
		mutable Mutex m_MessageQueueMutex;

		TArray<AssetWorker> m_AssetWorkers;

		WorkQueue m_WorkQueue;
		mutable Mutex m_WorkQueueMutex;
		mutable ConditionVariable m_WorkQueueCV;
		mutable ConditionVariable m_WaitForWorkersCV;

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
}
#include "AssetManager.inl"
