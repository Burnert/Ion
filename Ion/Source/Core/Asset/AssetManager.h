#pragma once

#include "AssetCore.h"
#include "Core/Memory/MemoryPool.h"

#define ASSET_WORKER_COUNT 4

#define DEFAULT_ASSET_ALIGNMENT   64 // Cache line size for concurrent use

// @TODO: Make these be in an engine config file

#define DEFAULT_MESH_POOL_SIZE    (1 << 27) // 128 MB
#define DEFAULT_TEXTURE_POOL_SIZE (1 << 29) // 512 MB

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
	using AssetWork_OnAllocErrorEvent = TFunction<void(AssetReference&, Memory::AllocError_Details&)>;

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

		MemoryPool* PoolPtr;
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
		void* AllocateAssetData(size_t size, Memory::AllocError_Details& outErrorData);

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

		static void Init();
		static void Update();
		static void Shutdown();

		template<EAssetType Type>
		static AssetHandle CreateAsset(FilePath location);
		static AssetHandle CreateAsset(EAssetType type, FilePath location);
		static void DeleteAsset(AssetHandle handle);

		//static const AssetReference* GetAssetReference(AssetHandle handle);

		static inline bool IsAssetLoaded(AssetHandle handle)
		{
			return GetAssetReference(handle)->IsLoaded();
		}

		static inline bool IsHandleValid(AssetHandle handle)
		{
			return handle.m_ID != INVALID_ASSET_ID &&
				m_Assets.find(handle.m_ID) != m_Assets.end();
		}

		template<typename ForEach>
		static void IterateMessages(ForEach forEach);

		template<EAssetType Type>
		static size_t GetAssetAlignment();

		template<EAssetType Type>
		static void DefragmentAssetPool();

		// This whole thing with the getters is a temporary solution
		// @TODO: Switch to static linking for god's sake...
		static MemoryPool& GetMeshAssetMemoryPool();
		static MemoryPool& GetTextureAssetMemoryPool();
		template<EAssetType Type>
		static MemoryPool& GetAssetMemoryPool();

		template<EAssetType Type>
		static void PrintAssetPool();

		template<EAssetType Type>
		static TShared<MemoryPoolDebugInfo> GetAssetPoolDebugInfo();

		// Static class
		AssetManager() = delete;
		~AssetManager() = delete;

	private:
		static AssetReference* GetAssetReference(AssetHandle handle);

		static void LoadAssetData(AssetReference& ref);
		static void UnloadAssetData(AssetReference& ref);

		static MemoryPool* GetAssetMemoryPool(EAssetType type);

	private:
		static void HandleAssetRealloc(void* oldLocation, void* newLocation);

		// Worker related

		template<typename T>
		static void ScheduleAssetWorkerWork(T& work);
		static void ScheduleLoadWork(AssetReference& ref);

		// Not thread-safe, lock m_WorkQueueMutex before use
		static bool IsAnyWorkerWorking();
		static void WaitForWorkers();

		// End of Worker related

		template<typename T>
		static void AddMessage(T& message);

		template<typename T>
		static void _DispatchMessage(T& message); // underscore because there's a Windows macro called DispatchMessage

		static void DispatchMessages();

		static void OnAssetLoaded(OnAssetLoadedMessage& message);
		static void OnAssetAllocError(OnAssetAllocErrorMessage& message);
		static void OnAssetUnloaded(OnAssetUnloadedMessage& message);
		static void OnAssetRealloc(OnAssetReallocMessage& message);

		static void ResolveErrors();
		static void ResolveAllocError(AssetReference& ref, Memory::AllocError_Details& details);

	private:
		static MemoryPool m_MeshAssetPool;
		static MemoryPool m_TextureAssetPool;

		static THashMap<AssetID, AssetReference> m_Assets;
		static THashMap<void*, AssetReference*> m_LoadedAssets;
		static TArray<AssetReference*> m_ErrorAssets;

		static MessageQueue m_MessageQueue;
		static Mutex m_MessageQueueMutex;

		static TArray<AssetWorker> m_AssetWorkers;

		static WorkQueue m_WorkQueue;
		static Mutex m_WorkQueueMutex;
		static ConditionVariable m_WorkQueueCV;
		static ConditionVariable m_WaitForWorkersCV;

		friend class AssetWorker;
		friend class AssetInterface;
		friend class Application;

		template<EAssetType Type>
		friend struct _TAssetPoolFromType;

	public:
		template<EAssetType Type>
		struct _TAssetPoolFromType;
		template<>
		struct _TAssetPoolFromType<EAssetType::Mesh>    { static inline constexpr MemoryPool& Ref = m_MeshAssetPool; };
		template<>
		struct _TAssetPoolFromType<EAssetType::Texture> { static inline constexpr MemoryPool& Ref = m_TextureAssetPool; };

		template<EAssetType Type>
		static inline constexpr MemoryPool& TAssetPoolFromType = _TAssetPoolFromType<Type>::Ref;
	};
}
#include "AssetManager.inl"
