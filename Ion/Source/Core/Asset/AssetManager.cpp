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
		return AssetManager::Get()->IsHandleValid(*this);
	}

	AssetReference::AssetReference() :
		Data(AssetData()),
		ID(INVALID_ASSET_ID),
		Type(EAssetType::Null),
		PackedFlags(0)
	{
		memset(ErrorData, 0, sizeof(ErrorData));
		//memset(Description, 0, sizeof(Description));
	}

	void AssetReference::CopyErrorData(const MemoryBlockDescriptor& data)
	{
		memcpy(ErrorData, data.Ptr, data.Size);
	}

	void AssetInterface::LoadAssetData()
	{
		AssetManager::Get()->LoadAssetData(*m_RefPtr);
	}

	void AssetInterface::UnloadAssetData()
	{
		AssetManager::Get()->UnloadAssetData(*m_RefPtr);
	}

	// AssetManager -------------------------------------------------

	void AssetManager::Init()
	{
		TRACE_FUNCTION();

		{
			TRACE_SCOPE("Allocate Mesh Asset Pool");
			m_MeshAssetPool.AllocatePool(DEFAULT_MESH_POOL_SIZE, DEFAULT_ASSET_ALIGNMENT);
		}
		{
			TRACE_SCOPE("Allocate Texture Asset Pool");
			m_TextureAssetPool.AllocatePool(DEFAULT_TEXTURE_POOL_SIZE, DEFAULT_ASSET_ALIGNMENT);
		}

		for (AssetWorker& worker : m_AssetWorkers)
		{
			worker = AssetWorker(this);
			worker.Start();
		}
	}

	void AssetManager::Update()
	{
		TRACE_FUNCTION();

		ResolveErrors();

		IterateMessages([](auto& msg)
		{
			_DispatchMessage(msg);
		});
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

		if (!ref.Data.Ptr)
		{
			return;
		}

		switch (ref.Type)
		{
			case EAssetType::Mesh:
			{
				m_MeshAssetPool.Free(ref.Data.Ptr);
				break;
			}
			case EAssetType::Texture:
			{
				m_TextureAssetPool.Free(ref.Data.Ptr);
				break;
			}
		}

		OnAssetUnloadedMessage message { };
		message.RefPtr = &ref;
		message.LastPoolLocation = ref.Data.Ptr;

		ref.Data.Ptr = nullptr;
		ref.Data.Size = 0;

		_DispatchMessage(message);
	}

	AssetMemoryPool* AssetManager::GetAssetMemoryPool(EAssetType type)
	{
		switch (type)
		{
			case EAssetType::Mesh:    return &m_MeshAssetPool;
			case EAssetType::Texture: return &m_TextureAssetPool;
		}
		return nullptr;
	}

	void AssetManager::HandleAssetRealloc(void* oldLocation, void* newLocation)
	{
		OnAssetReallocMessage message { };
		message.OldPoolLocation = oldLocation;
		message.NewPoolLocation = newLocation;
		message.RefPtr = m_LoadedAssets.at(oldLocation);

		_DispatchMessage(message);
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

	const AssetReference* AssetManager::GetAssetReference(AssetHandle handle) const
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
		work.OnLoad = [this](AssetReference& ref)
		{
			LOG_TRACE(L"Loaded asset \"{0}\"", ref.Location.ToString());
			
			OnAssetLoadedMessage message { };
			message.RefPtr = &ref;
			message.PoolLocation = ref.Data.Ptr;
			AddMessage(message);
		};
		work.OnLoadError = [this](AssetReference& ref)
		{
			LOG_ERROR(L"Could not load asset \"{0}\"", ref.Location.ToString());

			OnAssetLoadErrorMessage message { };
			message.RefPtr = &ref;
			message.ErrorMessage = ""; // @TODO: Add error message output
			AddMessage(message);
		};
		work.OnAllocError = [this](AssetReference& ref, AllocError_Details& details)
		{
			LOG_WARN(L"Could not allocate asset \"{0}\"", ref.Location.ToString());

			OnAssetAllocErrorMessage message { };
			message.RefPtr = &ref;
			message.AssetType = ref.Type;
			message.ErrorDetails = details;
			AddMessage(message);
		};
		ScheduleAssetWorkerWork(work);
	}

	void AssetManager::OnAssetLoaded(OnAssetLoadedMessage& message)
	{
		m_LoadedAssets.emplace(message.PoolLocation, message.RefPtr);
		message.RefPtr->bScheduledLoad = false;
	}

	void AssetManager::OnAssetAllocError(OnAssetAllocErrorMessage& message)
	{
		// Mark the asset to resolve the error later
		message.RefPtr->CopyErrorData(MemoryBlockDescriptor { &message.ErrorDetails, sizeof(message.ErrorDetails) });
		message.RefPtr->bAllocError = true;

		m_ErrorAssets.push_back(message.RefPtr);
	}

	void AssetManager::OnAssetUnloaded(OnAssetUnloadedMessage& message)
	{
		m_LoadedAssets.erase(message.LastPoolLocation);
	}

	void AssetManager::OnAssetRealloc(OnAssetReallocMessage& message)
	{
		auto node = m_LoadedAssets.extract(message.OldPoolLocation);
		node.mapped()->Data.Ptr = message.NewPoolLocation;
		node.key() = message.NewPoolLocation;
		m_LoadedAssets.insert(Move(node));
	}

	void AssetManager::ResolveErrors()
	{
		if (m_ErrorAssets.empty())
			return;

		// Resolve errors only if the workers don't have any work
		// and there are no messages left
		// @TODO: There's probably a better solution, like forcing the workers
		// to finish their work and making them wait for the resolution to finish
		{
			UniqueLock lock(m_WorkQueueMutex);
			if (!m_WorkQueue.empty())
				return;
			for (AssetWorker& worker : m_AssetWorkers)
			{
				if (worker.m_CurrentWork != EAssetWorkerWorkType::Null)
					return;
			}
		}{
			UniqueLock lock(m_MessageQueueMutex);
			if (!m_MessageQueue.empty())
				return;
		}
		
		TArray<AssetReference*> resolvedAssets;

		for (AssetReference* refPtr : m_ErrorAssets)
		{
			// Try to load again the assets which failed to allocate
			if (refPtr->bAllocError)
			{
				AllocError_Details errorDetails = *(AllocError_Details*)refPtr->ErrorData;

				LOG_TRACE(L"Resolving Asset Alloc Error (\"{0}\")", refPtr->Location.ToString());

				ResolveAllocError(*refPtr, errorDetails);
				refPtr->bAllocError = false;

				resolvedAssets.push_back(refPtr);
			}
		}

		m_ErrorAssets.clear();

		for (AssetReference* refPtr : resolvedAssets)
		{
			ScheduleLoadWork(*refPtr);
		}
	}

	void AssetManager::ResolveAllocError(AssetReference& ref, AllocError_Details& details)
	{
		AssetMemoryPool& poolRef = *GetAssetMemoryPool(ref.Type);

		auto onItemReallocFunc = [this](void* oldLocation, void* newLocation)
		{
			HandleAssetRealloc(oldLocation, newLocation);
		};

		if (!poolRef.CanAlloc(details.FailedAllocSize))
		{
			if (details.bPoolOutOfMemory)
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
		}

		if (details.bPoolFragmented)
		{
			poolRef.DefragmentPool(onItemReallocFunc);
		}
	}

	AssetManager::AssetManager() :
		m_AssetWorkers(ASSET_WORKER_COUNT)
	{
	}

	// AssetWorker ---------------------------------------------------

	AssetWorker::AssetWorker() :
		AssetWorker(nullptr)
	{
	}

	AssetWorker::AssetWorker(AssetManager* owner) :
		m_Owner(owner),
		m_bExit(false),
		m_CurrentWork(EAssetWorkerWorkType::Null)
	{
	}

	void AssetWorker::Start()
	{
		ionassertnd(m_Owner);
		m_WorkerThread = Thread(&AssetWorker::WorkerProc, this);
	}

	void AssetWorker::Exit()
	{
		UniqueLock lock(m_Owner->m_WorkQueueMutex);
		m_bExit = true;
	}

	// Worker Thread:

	void AssetWorker::WorkerProc()
	{
		SetThreadDescription(GetCurrentThread(), L"AssetWorker");

		AssetManager::WorkQueue& queue = m_Owner->m_WorkQueue;

		TShared<AssetWorkerWorkBase> work;

		while (true)
		{
			{
				UniqueLock lock(m_Owner->m_WorkQueueMutex);

				m_CurrentWork = EAssetWorkerWorkType::Null;

				// Wait for work
				m_Owner->m_WorkQueueCV.wait(lock, [this, &queue]
				{
					return queue.size() || m_bExit;
				});

				if (m_bExit)
				{
					break;
				}

				// Move the work data
				work = queue.front();
				queue.pop();

				m_CurrentWork = work->GetType();
			}

			DispatchWork(work);
			work.reset();
		}
	}

#define _DISPATCH_WORK_CASE(name) \
	case EAssetWorkerWorkType::name: \
		Do##name##Work(TStaticCast<AssetWorker##name##Work>(work)); break

	void AssetWorker::DispatchWork(const TShared<AssetWorkerWorkBase>& work)
	{
		switch (work->GetType())
		{
			_DISPATCH_WORK_CASE(Load);
			_DISPATCH_WORK_CASE(Realloc);
		}
	}

#define HANDLE_LOAD_ERROR(x) \
	if (!(x)) { \
		work->OnLoadError(*refPtr); \
		return; \
	}

#define HANDLE_ALLOC_ERROR(x, errorDetails) \
	if (!(x)) { \
		work->OnAllocError(*refPtr, errorDetails); \
		return; \
	}

	void AssetWorker::DoLoadWork(const TShared<AssetWorkerLoadWork>& work)
	{
		LOG_TRACE(L"Loading asset \"{0}\"...", work->RefPtr->Location.ToString());

		bool bResult;
		AssetReference* refPtr = work->RefPtr;

		File file(refPtr->Location);

		void* poolBlockPtr = nullptr;
		size_t poolBlockSize = 0;
		
		switch (refPtr->Type)
		{
			case EAssetType::Mesh:
			{
				// @TODO: Same as textures

				String collada;
				bResult = file.Open(EFileMode::Read);

				HANDLE_LOAD_ERROR(bResult);

				bResult = file.Read(collada);

				HANDLE_LOAD_ERROR(bResult);

				file.Close();

				TUnique<ColladaDocument> mesh = MakeUnique<ColladaDocument>(collada);
				const ColladaData& meshData = mesh->GetData();

				size_t vertexTypeSize = sizeof(TRemovePtr<decltype(meshData.VertexAttributes)>);
				size_t indexTypeSize  = sizeof(TRemovePtr<decltype(meshData.Indices)>);

				uint64 vertexCount = meshData.VertexAttributeCount;
				uint64 indexCount = meshData.IndexCount;

				AssetTypes::MeshDesc& desc = *(refPtr->GetDescription<EAssetType::Mesh>());
				desc.VertexCount = vertexCount;
				desc.IndexCount = indexCount;
				desc.VertexLayout = meshData.Layout;

				size_t alignment = m_Owner->GetAssetAlignment<EAssetType::Mesh>();
				size_t vertexBlockSize = AlignAs(vertexCount * vertexTypeSize, alignment);
				size_t indexBlockSize  = AlignAs(indexCount * indexTypeSize, alignment);
				poolBlockSize = vertexBlockSize + indexBlockSize;
				size_t& indexBlockOffset = vertexBlockSize;

				AllocError_Details errorDetails { };
				poolBlockPtr = AllocateAssetData<EAssetType::Mesh>(poolBlockSize, errorDetails);
				HANDLE_ALLOC_ERROR(poolBlockPtr, errorDetails);

				desc.VerticesOffset = 0;
				desc.IndicesOffset = indexBlockOffset;

				memcpy(poolBlockPtr, meshData.VertexAttributes, vertexTypeSize * vertexCount);
				memcpy((uint8*)poolBlockPtr + indexBlockOffset, meshData.Indices, indexTypeSize * indexCount);

				break;
			}
			case EAssetType::Texture:
			{
				// @TODO: This will have to use an optimized texture format instead of plain pixel data

				Image image;
				const uint8* pixelData = image.Load(file);

				HANDLE_LOAD_ERROR(pixelData);

				AssetTypes::TextureDesc& desc = *(refPtr->GetDescription<EAssetType::Texture>());
				desc.Width = image.GetWidth();
				desc.Height = image.GetHeight();
				desc.NumChannels = image.GetChannelNum();
				desc.BytesPerChannel = 1;

				poolBlockSize = image.GetPixelDataSize();

				AllocError_Details errorDetails { };
				poolBlockPtr = AllocateAssetData<EAssetType::Texture>(poolBlockSize, errorDetails);
				HANDLE_ALLOC_ERROR(poolBlockPtr, errorDetails);

				memcpy(poolBlockPtr, pixelData, poolBlockSize);

				break;
			}
			default:
			{
				std::stringstream ss;
				ss << std::this_thread::get_id();
				LOG_WARN("Unknown asset type in worker thread (TID: {0})", ss.str());

				HANDLE_LOAD_ERROR(false);
			}
		}

		refPtr->Data.Ptr = poolBlockPtr;
		refPtr->Data.Size = poolBlockSize;

		work->OnLoad(*refPtr);
	}

	void AssetWorker::DoReallocWork(const TShared<AssetWorkerReallocWork>& work)
	{

	}
	// End of Worker Thread
}
