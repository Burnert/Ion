#include "IonPCH.h"

#include "AssetManager.h"
#include "Core/File/Collada.h"

namespace Ion
{
	// AssetCore definitions ----------------------------------------

	AssetID g_CurrentAssetID = 0;
	AssetID CreateAssetID() { return g_CurrentAssetID++; }

	bool AssetHandle::IsValid() const
	{
		return AssetManager::IsHandleValid(*this);
	}

	AssetReference::AssetReference() :
		Data(AssetData()),
		ID(INVALID_ASSET_ID),
		Type(EAssetType::Null),
		PackedFlags(0),
		ErrorData()
	{
	}

	void AssetReference::SetErrorData(const MemoryBlock& data)
	{
		TRACE_FUNCTION();

		ionassert(data.Ptr);
		ionassert(data.Size);
		ionassert(data.Size <= sizeof(ErrorData));

		memcpy(ErrorData, data.Ptr, data.Size);
	}

	void AssetInterface::LoadAssetData()
	{
		TRACE_FUNCTION();

		ionassert(m_RefPtr);

		AssetManager::LoadAssetData(*m_RefPtr);
	}

	void AssetInterface::UnloadAssetData()
	{
		TRACE_FUNCTION();

		ionassert(m_RefPtr);

		AssetManager::UnloadAssetData(*m_RefPtr);
	}

	// AssetManager -------------------------------------------------

	void AssetManager::Init()
	{
		TRACE_FUNCTION();

		{
			TRACE_SCOPE("Allocate Mesh Asset Pool");
			m_MeshAssetPool.AllocPool(DEFAULT_MESH_POOL_SIZE, DEFAULT_ASSET_ALIGNMENT);
		}
		{
			TRACE_SCOPE("Allocate Texture Asset Pool");
			m_TextureAssetPool.AllocPool(DEFAULT_TEXTURE_POOL_SIZE, DEFAULT_ASSET_ALIGNMENT);
		}

		for (AssetWorker& worker : m_AssetWorkers)
		{
			worker.Start();
		}
	}

	void AssetManager::Update()
	{
		TRACE_FUNCTION();

		DispatchMessages();

		ResolveErrors();
	}

	void AssetManager::Shutdown()
	{
		TRACE_FUNCTION();

		for (AssetWorker& worker : m_AssetWorkers)
		{
			worker.Exit();
		}
		m_WorkQueueCV.notify_all();

		for (AssetWorker& worker : m_AssetWorkers)
		{
			if (worker.m_WorkerThread.joinable())
				worker.m_WorkerThread.join();
		}

		m_MeshAssetPool.FreePool();
		m_TextureAssetPool.FreePool();
	}

	AssetHandle AssetManager::CreateAsset(EAssetType type, FilePath location)
	{
		TRACE_FUNCTION();

		switch (type)
		{
			case EAssetType::Mesh:
			{
				return CreateAsset<EAssetType::Mesh>(location);
			}
			case EAssetType::Texture:
			{
				return CreateAsset<EAssetType::Texture>(location);
			}
			default:
			{
				LOG_ERROR("Unsupported asset type!");
				return INVALID_ASSET_HANDLE;
			}
		}
	}

	void AssetManager::DeleteAsset(AssetHandle handle)
	{
		TRACE_FUNCTION();

		if (m_Assets.find(handle.m_ID) == m_Assets.end())
		{
			LOG_WARN("Asset {0} does not exist.", handle.m_ID);
			return;
		}

		AssetReference& ref = m_Assets.at(handle.m_ID);

		UnloadAssetData(ref);

		m_Assets.erase(handle.m_ID);
	}

	inline void AssetManager::LoadAssetData(AssetReference& ref)
	{
		TRACE_FUNCTION();

		if (!ref.IsLoaded() && !ref.IsLoading())
		{
			ref.bScheduledLoad = true;
			ScheduleLoadWork(ref);
		}
	}

	void AssetManager::UnloadAssetData(AssetReference& ref)
	{
		TRACE_FUNCTION();

		MemoryPool* poolPtr = GetAssetMemoryPool(ref.Type);
		ionassert(poolPtr);

		if (!ref.Data.Ptr)
		{
			return;
		}

		poolPtr->Free(ref.Data.Ptr);

		OnAssetUnloadedMessage message { };
		message.RefPtr = &ref;
		message.LastPoolLocation = ref.Data.Ptr;

		ref.Data.Ptr = nullptr;
		ref.Data.Size = 0;

		_DispatchMessage(message);
	}

	MemoryPool* AssetManager::GetAssetMemoryPool(EAssetType type)
	{
		switch (type)
		{
			case EAssetType::Mesh:    return &m_MeshAssetPool;
			case EAssetType::Texture: return &m_TextureAssetPool;
		}
		ionassertnd(false, "An Asset Memory Pool of that type does not exist.");
		return nullptr;
	}

	void AssetManager::HandleAssetRealloc(void* oldLocation, void* newLocation)
	{
		TRACE_FUNCTION();

		OnAssetReallocMessage message { };
		message.OldPoolLocation = oldLocation;
		message.NewPoolLocation = newLocation;
		message.RefPtr = m_LoadedAssets.at(oldLocation);

		_DispatchMessage(message);
	}

	MemoryPool& AssetManager::GetMeshAssetMemoryPool()
	{
		return m_MeshAssetPool;
	}

	MemoryPool& AssetManager::GetTextureAssetMemoryPool()
	{
		return m_TextureAssetPool;
	}

	AssetReference* AssetManager::GetAssetReference(AssetHandle handle)
	{
		TRACE_FUNCTION();

		if (m_Assets.find(handle.m_ID) == m_Assets.end())
		{
			LOG_ERROR("Invalid AssetHandle!");
			return nullptr;
		}
		return &m_Assets.at(handle.m_ID);
	}

	void AssetManager::ScheduleLoadWork(AssetReference& ref)
	{
		TRACE_FUNCTION();

		AssetWorkerLoadWork work { };
		work.RefPtr = &ref;
		work.OnLoad = [](AssetReference& ref)
		{
			TRACE_SCOPE("AssetManager::ScheduleLoadWork - OnLoad");

			LOG_TRACE(L"Loaded asset \"{0}\"", ref.Location.ToString());
			
			OnAssetLoadedMessage message { };
			message.RefPtr = &ref;
			message.PoolLocation = ref.Data.Ptr;
			AddMessage(message);
		};
		work.OnLoadError = [](AssetReference& ref)
		{
			TRACE_SCOPE("AssetManager::ScheduleLoadWork - OnLoadError");

			LOG_ERROR(L"Could not load asset \"{0}\"", ref.Location.ToString());

			OnAssetLoadErrorMessage message { };
			message.RefPtr = &ref;
			message.ErrorMessage = ""; // @TODO: Add error message output
			AddMessage(message);
		};
		work.OnAllocError = [](AssetReference& ref, Memory::AllocError_Details& details)
		{
			TRACE_SCOPE("AssetManager::ScheduleLoadWork - OnAllocError");

			LOG_WARN(L"Could not allocate asset \"{0}\"", ref.Location.ToString());

			OnAssetAllocErrorMessage message { };
			message.RefPtr = &ref;
			message.AssetType = ref.Type;
			message.ErrorDetails = details;
			AddMessage(message);
		};
		ScheduleAssetWorkerWork(work);
	}

	// Not thread-safe, lock m_WorkQueueMutex before use
	bool AssetManager::IsAnyWorkerWorking()
	{
		TRACE_FUNCTION();

		for (const AssetWorker& worker : m_AssetWorkers)
		{
			if (worker.m_CurrentWork != EAssetWorkerWorkType::Null)
				return true;
		}
		return false;
	}

	void AssetManager::WaitForWorkers()
	{
		TRACE_FUNCTION();

		UniqueLock lock(m_WorkQueueMutex);

		m_WaitForWorkersCV.wait(lock, []
		{
			return !IsAnyWorkerWorking();
		});
	}

	void AssetManager::DispatchMessages()
	{
		TRACE_FUNCTION();

		IterateMessages([](auto& msg)
		{
			_DispatchMessage(msg);
		});
	}

	void AssetManager::OnAssetLoaded(OnAssetLoadedMessage& message)
	{
		TRACE_FUNCTION();

		m_LoadedAssets.emplace(message.PoolLocation, message.RefPtr);
		message.RefPtr->bScheduledLoad = false;
	}

	void AssetManager::OnAssetAllocError(OnAssetAllocErrorMessage& message)
	{
		TRACE_FUNCTION();

		// Mark the asset to resolve the error later
		message.RefPtr->SetErrorData(MemoryBlock { &message.ErrorDetails, sizeof(message.ErrorDetails) });
		message.RefPtr->bAllocError = true;

		m_ErrorAssets.push_back(message.RefPtr);
	}

	void AssetManager::OnAssetUnloaded(OnAssetUnloadedMessage& message)
	{
		TRACE_FUNCTION();

		m_LoadedAssets.erase(message.LastPoolLocation);
	}

	void AssetManager::OnAssetRealloc(OnAssetReallocMessage& message)
	{
		TRACE_FUNCTION();

		auto node = m_LoadedAssets.extract(message.OldPoolLocation);
		node.mapped()->Data.Ptr = message.NewPoolLocation;
		node.key() = message.NewPoolLocation;
		m_LoadedAssets.insert(Move(node));
	}

	void AssetManager::ResolveErrors()
	{
		TRACE_FUNCTION();

		if (m_ErrorAssets.empty())
			return;

		WorkQueue currentWork;
		{
			TRACE_SCOPE("AssetManager::ResolveErrors - Wait");
			// Temporarily remove any scheduled work and wait
			// for the workers to finish their remaining jobs
			{
				UniqueLock lock(m_WorkQueueMutex);
				m_WorkQueue.swap(currentWork);
			}
			WaitForWorkers();
			// Handle any messages that were generated, before continuing
			{
				UniqueLock lock(m_MessageQueueMutex);

				if (!m_MessageQueue.empty())
				{
					lock.unlock();
					DispatchMessages();
				}
			}
		}
		{
			TRACE_SCOPE("AssetManager::ResolveErrors - Resolve");

			TArray<AssetReference*> resolvedAssets;
			resolvedAssets.reserve(m_ErrorAssets.size());

			for (AssetReference* refPtr : m_ErrorAssets)
			{
				// Try to load again the assets which failed to allocate
				if (refPtr->bAllocError)
				{
					LOG_TRACE(L"Resolving Asset Alloc Error (\"{0}\")", refPtr->Location.ToString());

					Memory::AllocError_Details errorDetails = *(Memory::AllocError_Details*)refPtr->ErrorData;

					ResolveAllocError(*refPtr, errorDetails);
					refPtr->bAllocError = false;

					resolvedAssets.push_back(refPtr);
				}
			}

			m_ErrorAssets.clear();

			// Add the work back to the queue
			{
				UniqueLock lock(m_WorkQueueMutex);
				m_WorkQueue.swap(currentWork);
			}
			m_WorkQueueCV.notify_all();

			// Schedule another load for the resolved assets
			for (AssetReference* refPtr : resolvedAssets)
			{
				ScheduleLoadWork(*refPtr);
			}
		}
	}

	void AssetManager::ResolveAllocError(AssetReference& ref, Memory::AllocError_Details& details)
	{
		TRACE_FUNCTION();

		MemoryPool& poolRef = *GetAssetMemoryPool(ref.Type);

		auto onItemReallocFunc = [](void* oldLocation, void* newLocation)
		{
			HandleAssetRealloc(oldLocation, newLocation);
		};

		if (details.bPoolOutOfMemory &&
			!poolRef.CanAlloc(details.FailedAllocSize))
		{
			// @TODO: Set some limit to how large the pool can be
			size_t newPoolSize = details.bAllocSizeGreaterThanPoolSize ?
				// Align to 64 KB
				// Assume there will be more assets of this size
				AlignAs(details.FailedAllocSize * 4, (1 << 16)) :
				// If just out of memory, resize to twice the size
				poolRef.GetSize() * 2;

			LOG_INFO("Reallocating {0} Pool - New Size: {1} B", AssetTypeToString(ref.Type), newPoolSize);
			poolRef.ReallocPool(newPoolSize, onItemReallocFunc);
		}

		if (details.bPoolFragmented)
		{
			poolRef.DefragmentPool(onItemReallocFunc);
		}
	}

	MemoryPool AssetManager::m_MeshAssetPool;
	MemoryPool AssetManager::m_TextureAssetPool;

	THashMap<AssetID, AssetReference> AssetManager::m_Assets;
	THashMap<void*, AssetReference*> AssetManager::m_LoadedAssets;
	TArray<AssetReference*> AssetManager::m_ErrorAssets;

	AssetManager::MessageQueue AssetManager::m_MessageQueue;
	Mutex AssetManager::m_MessageQueueMutex;

	TArray<AssetWorker> AssetManager::m_AssetWorkers(ASSET_WORKER_COUNT);

	AssetManager::WorkQueue AssetManager::m_WorkQueue;
	Mutex AssetManager::m_WorkQueueMutex;
	ConditionVariable AssetManager::m_WorkQueueCV;
	ConditionVariable AssetManager::m_WaitForWorkersCV;

	// AssetWorker ---------------------------------------------------

	AssetWorker::AssetWorker() :
		m_bExit(false),
		m_CurrentWork(EAssetWorkerWorkType::Null)
	{
	}

	void AssetWorker::Start()
	{
		m_WorkerThread = Thread(&AssetWorker::WorkerProc, this);
	}

	void AssetWorker::Exit()
	{
		UniqueLock lock(AssetManager::m_WorkQueueMutex);
		m_bExit = true;
	}

	// Worker Thread:

	void AssetWorker::WorkerProc()
	{
		TRACE_FUNCTION();

		SetThreadDescription(GetCurrentThread(), L"AssetWorker");

		AssetManager::WorkQueue& queue = AssetManager::m_WorkQueue;

		TShared<AssetWorkerWorkBase> work;

		while (true)
		{
			{
				UniqueLock lock(AssetManager::m_WorkQueueMutex);
				{
					TRACE_SCOPE("AssetWorker::WorkerProc - Wait for work");
					// Wait for work
					AssetManager::m_WorkQueueCV.wait(lock, [this, &queue]
					{
						return queue.size() || m_bExit;
					});
				}

				if (m_bExit)
				{
					break;
				}

				// Move the work data
				work = queue.front();
				queue.pop();

				m_CurrentWork = work->GetType();
			}
			TRACE_SCOPE("AssetWorker::WorkerProc - Work loop");

			DispatchWork(work);
			work.reset();
			{
				UniqueLock lock(AssetManager::m_WorkQueueMutex);
				m_CurrentWork = EAssetWorkerWorkType::Null;
			}
			AssetManager::m_WaitForWorkersCV.notify_one();
		}
	}

#define _DISPATCH_WORK_CASE(name) \
	case EAssetWorkerWorkType::name: \
		Do##name##Work(TStaticCast<AssetWorker##name##Work>(work)); break

	void AssetWorker::DispatchWork(const TShared<AssetWorkerWorkBase>& work)
	{
		TRACE_FUNCTION();

		switch (work->GetType())
		{
			_DISPATCH_WORK_CASE(Load);
			_DISPATCH_WORK_CASE(Realloc);
		}
	}

#define HANDLE_LOAD_ERROR(x) \
	if (!(x)) { \
		work->OnLoadError(*work->RefPtr); \
		return false; \
	}

#define HANDLE_ALLOC_ERROR(x, errorDetails) \
	if (!(x)) { \
		work->OnAllocError(*work->RefPtr, errorDetails); \
		return false; \
	}

	void AssetWorker::DoLoadWork(const TShared<AssetWorkerLoadWork>& work)
	{
		TRACE_FUNCTION();

		LOG_TRACE(L"Loading asset \"{0}\"...", work->RefPtr->Location.ToString());

		AssetReference* refPtr = work->RefPtr;

		File file(refPtr->Location);

		void* poolBlockPtr = nullptr;
		size_t poolBlockSize = 0;
		
		switch (refPtr->Type)
		{
			case EAssetType::Mesh:
			{
				if (!LoadMesh(file, work, &poolBlockPtr, &poolBlockSize))
					return;
				break;
			}
			case EAssetType::Texture:
			{
				if (!LoadTexture(file, work, &poolBlockPtr, &poolBlockSize))
					return;
				break;
			}
			default:
			{
				std::stringstream ss;
				ss << std::this_thread::get_id();
				LOG_WARN("Unknown asset type in worker thread (TID: {0})", ss.str());
				
				work->OnLoadError(*refPtr);
				return;
			}
		}

		refPtr->Data.Ptr = poolBlockPtr;
		refPtr->Data.Size = poolBlockSize;

		work->OnLoad(*refPtr);
	}

	void AssetWorker::DoReallocWork(const TShared<AssetWorkerReallocWork>& work)
	{

	}

	bool AssetWorker::LoadMesh(File& file, const TShared<AssetWorkerLoadWork>& work, void** outDataPtr, size_t* outDataSize)
	{
		TRACE_FUNCTION();

		// @TODO: Same as textures

		void* poolBlockPtr;
		size_t poolBlockSize;

		String collada;

		HANDLE_LOAD_ERROR(file.Open(EFileMode::Read));
		HANDLE_LOAD_ERROR(file.Read(collada));

		file.Close();

		TUnique<ColladaDocument> mesh = MakeUnique<ColladaDocument>(collada);
		const ColladaData& meshData = mesh->GetData();

		size_t vertexTypeSize = sizeof(TRemovePtr<decltype(meshData.VertexAttributes)>);
		size_t indexTypeSize = sizeof(TRemovePtr<decltype(meshData.Indices)>);

		uint64 vertexCount = meshData.VertexAttributeCount;
		uint64 indexCount = meshData.IndexCount;

		AssetDescription::Mesh& desc = *(work->RefPtr->GetDescription<EAssetType::Mesh>());
		desc.VertexCount = vertexCount;
		desc.IndexCount = indexCount;
		desc.VertexLayout = meshData.Layout;

		size_t alignment = AssetManager::GetAssetAlignment<EAssetType::Mesh>();
		size_t vertexBlockSize = AlignAs(vertexCount * vertexTypeSize, alignment);
		size_t indexBlockSize = AlignAs(indexCount * indexTypeSize, alignment);
		poolBlockSize = vertexBlockSize + indexBlockSize;
		size_t& indexBlockOffset = vertexBlockSize;

		Memory::AllocError_Details errorDetails { };
		poolBlockPtr = AllocateAssetData<EAssetType::Mesh>(poolBlockSize, errorDetails);
		HANDLE_ALLOC_ERROR(poolBlockPtr, errorDetails);

		desc.VerticesOffset = 0;
		desc.IndicesOffset = indexBlockOffset;

		memcpy(poolBlockPtr, meshData.VertexAttributes, vertexTypeSize * vertexCount);
		memcpy((uint8*)poolBlockPtr + indexBlockOffset, meshData.Indices, indexTypeSize * indexCount);

		*outDataPtr = poolBlockPtr;
		*outDataSize = poolBlockSize;

		return true;
	}

	bool AssetWorker::LoadTexture(File& file, const TShared<AssetWorkerLoadWork>& work, void** outDataPtr, size_t* outDataSize)
	{
		TRACE_FUNCTION();

		// @TODO: This will have to use an optimized texture format instead of plain pixel data

		void* poolBlockPtr;
		size_t poolBlockSize;

		Image image;
		const uint8* pixelData = image.Load(file);

		HANDLE_LOAD_ERROR(pixelData);

		AssetDescription::Texture& desc = *(work->RefPtr->GetDescription<EAssetType::Texture>());
		desc.Width = image.GetWidth();
		desc.Height = image.GetHeight();
		desc.NumChannels = image.GetChannelNum();
		desc.BytesPerChannel = 1;

		poolBlockSize = image.GetPixelDataSize();

		Memory::AllocError_Details errorDetails { };
		poolBlockPtr = AllocateAssetData<EAssetType::Texture>(poolBlockSize, errorDetails);
		HANDLE_ALLOC_ERROR(poolBlockPtr, errorDetails);

		memcpy(poolBlockPtr, pixelData, poolBlockSize);

		*outDataPtr = poolBlockPtr;
		*outDataSize = poolBlockSize;

		return true;
	}

	// End of Worker Thread
}
