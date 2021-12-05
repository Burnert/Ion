#include "IonPCH.h"

#include "AssetManager.h"
#include "Core/File/Collada.h"

#define SLEEP_FOREVER() std::this_thread::sleep_for(std::chrono::hours(TNumericLimits<uint64>::max()))

namespace Ion
{
	// AssetCore definitions ----------------------------------------

	AssetID g_CurrentAssetID = 0;
	AssetID CreateAssetID() { return g_CurrentAssetID++; }

	bool AssetHandle::IsValid() const
	{
		return AssetManager::Get()->IsHandleValid(*this);
	}

	bool AssetHandle::IsLoaded() const
	{
		return AssetManager::Get()->IsAssetLoaded(*this);
	}

	AssetReference::AssetReference() :
		Data(AssetData()),
		ID(INVALID_ASSET_ID),
		Type(EAssetType::Null)
	{
		//memset(Description, 0, sizeof(Description));
	}

	void AssetInterface::LoadAssetData()
	{
		AssetManager::Get()->LoadAssetData(*m_RefPtr);
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

		for (AssetWorker& worker : m_WorkerThreads)
		{
			worker = AssetWorker(this);
			worker.Start();
		}
	}

#define IF_ASSET_MESSAGE_TYPE(msg, Type) \
if constexpr (TIsSameV<TRemoveRef<decltype(msg)>, Type>)

	void AssetManager::Update()
	{
		IterateMessages([](auto& msg)
		{
			_DispatchMessage(msg);
		});
	}

	void AssetManager::Shutdown()
	{
		TRACE_FUNCTION();

		for (AssetWorker& worker : m_WorkerThreads)
		{
			UniqueLock lock(m_WorkQueueMutex);
			worker.Exit();
		}
		m_WorkQueueCV.notify_all();

		for (AssetWorker& worker : m_WorkerThreads)
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

		UnloadAsset(ref);

		m_Assets.erase(handle.m_ID);
	}

	inline void AssetManager::LoadAssetData(AssetReference& ref)
	{
		if (!ref.Data.Ptr)
		{
			ScheduleAssetLoadWork(ref);
		}
		else
		{
			if (ref.Events.OnAssetLoaded)
			{
				OnAssetLoadedMessage message { };
				message.RefPtr = &ref;

				ref.Events.OnAssetLoaded(message);
			}
		}
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

	void AssetManager::ScheduleAssetLoadWork(AssetReference& ref)
	{
		TRACE_FUNCTION();

		AssetWorkerWork work { };
		work.RefPtr = &ref;
		work.OnLoad = [this](AssetReference& ref)
		{
			LOG_INFO(L"Loaded asset \"{0}\"", ref.Location.ToString());
			
			OnAssetLoadedMessage message { };
			message.RefPtr = &ref;
			AddMessage(message);
		};
		work.OnError = [this](AssetReference& ref)
		{
			LOG_ERROR(L"Could not load asset \"{0}\"", ref.Location.ToString());

			OnAssetLoadErrorMessage message { };
			message.RefPtr = &ref;
			message.ErrorMessage = ""; // @TODO: Add error message output
			AddMessage(message);
		};
		{
			UniqueLock lock(m_WorkQueueMutex);
			m_WorkQueue.emplace(Move(work));
		}
		m_WorkQueueCV.notify_one();
	}

	void AssetManager::UnloadAsset(AssetReference& ref)
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
	}

	AssetManager::AssetManager()
	{
	}

	// AssetWorker ---------------------------------------------------

	AssetWorker::AssetWorker() :
		AssetWorker(nullptr)
	{
	}

	AssetWorker::AssetWorker(AssetManager* owner) :
		m_Owner(owner),
		m_bExit(false)
	{
	}

	void AssetWorker::Start()
	{
		ionassertnd(m_Owner);
		m_WorkerThread = Thread(&AssetWorker::WorkerProc, this);
	}

	void AssetWorker::Exit()
	{
		m_bExit = true;
	}

	void AssetWorker::WorkerProc()
	{
		SetThreadDescription(GetCurrentThread(), L"AssetWorker");

		AssetManager::WorkerQueue& queue = AssetManager::Get()->m_WorkQueue;

		AssetWorkerWork work;

		while (true)
		{
			{
				UniqueLock lock(m_Owner->m_WorkQueueMutex);

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
				work = Move(queue.front());
				queue.pop();
			}

			if (LoadAsset(work.RefPtr))
			{
				work.OnLoad(*work.RefPtr);
			}
			else
			{
				work.OnError(*work.RefPtr);
			}
		}
	}

#define HANDLE_LOAD_ERROR(x) if (!(x)) return false

	bool AssetWorker::LoadAsset(AssetReference* refPtr)
	{
		bool bResult;

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

				poolBlockPtr = m_Owner->AllocateAssetData<EAssetType::Mesh>(poolBlockSize);

				HANDLE_LOAD_ERROR(poolBlockPtr);

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

				poolBlockPtr = m_Owner->AllocateAssetData<EAssetType::Texture>(poolBlockSize);

				HANDLE_LOAD_ERROR(poolBlockPtr);

				memcpy(poolBlockPtr, pixelData, poolBlockSize);

				break;
			}
			default:
				// @TODO: Print thread id
				LOG_WARN("Unknown asset type in worker thread ({0})", 0);
		}

		refPtr->Data.Ptr = poolBlockPtr;
		refPtr->Data.Size = poolBlockSize;

		return true;
	}
}
