#include "IonPCH.h"

#include "AssetManager.h"

#define SLEEP_FOREVER() std::this_thread::sleep_for(std::chrono::hours(TNumericLimits<uint64>::max()))

namespace Ion
{
	// AssetCore definitions ----------------------------------------

	AssetID g_CurrentAssetID = 0;
	AssetID CreateAssetID() { return g_CurrentAssetID++; }

	const AssetReference* AssetHandle::operator->() const
	{
		return AssetManager::Get()->GetAssetReference(*this);
	}

	bool AssetHandle::IsValid() const
	{
		return AssetManager::Get()->IsHandleValid(*this);
	}

	// AssetManager -------------------------------------------------

	void AssetManager::Init()
	{
		TRACE_FUNCTION();

		m_TextureAssetPool.AllocatePool(DEFAULT_TEXTURE_POOL_SIZE, DEFAULT_ASSET_ALIGNMENT);

		for (int32 i = 0; i < ASSET_WORKER_COUNT; ++i)
		{
			AssetWorker& worker = m_WorkerThreads.emplace_back(this);
		}
		// Second loop because m_Owner is junk in the worker thread
		// if I start it right after creating for some reason???
		for (int32 i = 0; i < ASSET_WORKER_COUNT; ++i)
		{
			AssetWorker& worker = m_WorkerThreads[i];
			worker.Start();
		}
	}

	void AssetManager::Shutdown()
	{
		TRACE_FUNCTION();

		m_TextureAssetPool.FreePool();
	}

	AssetReference& AssetManager::CreateAsset(EAssetType type, FilePath location)
	{
		TRACE_FUNCTION();

		ionassert(location.Exists());

		AssetReference asset { };
		asset.ID = CreateAssetID();
		asset.Location = location;
		asset.Type = type;

		void* desc = nullptr;
		size_t descSize = 0;

		switch (type)
		{
			case EAssetType::Texture:
			{
				desc = new AssetTypes::TextureDesc;
				descSize = sizeof(AssetTypes::TextureDesc);
				break;
			}
			default:
			{
				ionassertnd("Unsupported asset type!");
			}
		}

		asset.Description = desc;
		memset(asset.Description, 0, descSize);

		AssetReference& assetRef = m_Assets.emplace(asset.ID, Move(asset)).first->second;
		ScheduleAssetLoadWork(assetRef);

		return assetRef;
	}

	void AssetManager::DeleteAsset(AssetHandle handle)
	{
		TRACE_FUNCTION();

		ionassert(m_Assets.find(handle.ID) != m_Assets.end());

		AssetReference& ref = m_Assets.at(handle.ID);

		if (ref.Data.Ptr)
		{
			switch (ref.Type)
			{
				case EAssetType::Texture:
				{
					m_TextureAssetPool.Free(ref.Data.Ptr);
					break;
				}
			}
		}

		if (ref.Description)
			delete ref.Description;

		m_Assets.erase(handle.ID);
	}

	uint64 AssetManager::GetAssetSize(AssetHandle handle)
	{
		return handle->Data.Size;
	}

	AssetData AssetManager::GetAssetDataImmediate(AssetHandle handle)
	{
		return handle->Data;
	}

	AssetReference* AssetManager::GetAssetReference(AssetHandle handle)
	{
		TRACE_FUNCTION();

		if (m_Assets.find(handle.ID) == m_Assets.end())
		{
			LOG_ERROR("Invalid AssetHandle!");
			return nullptr;
		}
		return &m_Assets.at(handle.ID);
	}

	void AssetManager::ScheduleAssetLoadWork(AssetReference& ref)
	{
		TRACE_FUNCTION();

		AssetWorkerWork work { };
		work.RefPtr = &ref;
		work.OnLoad = [this](AssetReference& ref)
		{
			LOG_INFO(L"Loaded asset {0}", ref.Location.ToString());
		};

		UniqueLock lock(m_QueueMutex);
		m_WorkQueue.emplace(Move(work));
		lock.unlock();
		m_QueueCV.notify_one();
	}

	AssetManager::AssetManager()
	{

	}

	// AssetWorker ---------------------------------------------------


	AssetWorker::AssetWorker(AssetManager* owner) :
		m_Owner(owner),
		m_WorkerThread(Thread())
	{

	}

	void AssetWorker::Start()
	{
		m_WorkerThread = Thread(&AssetWorker::WorkerProc, this);
	}

	void AssetWorker::WorkerProc()
	{
		SetThreadDescription(GetCurrentThread(), L"AssetWorker");

		AssetManager::WorkerQueue& queue = AssetManager::Get()->m_WorkQueue;

		UniqueLock lock(m_Owner->m_QueueMutex);

		while (true) // @TODO: Add an exit condition
		{
			// Wait for work
			m_Owner->m_QueueCV.wait(lock, [&queue] {
				return queue.size();
			});

			// Move the work data
			AssetWorkerWork work = Move(queue.front());
			queue.pop();

			lock.unlock();

			LoadAsset(work.RefPtr->Type, work.RefPtr->Location, work.RefPtr);
			work.OnLoad(*work.RefPtr);

			// Lock again for the CV.wait
			lock.lock();
		}
	}

	void AssetWorker::LoadAsset(EAssetType type, FilePath location, AssetReference* refPtr)
	{
		File file(location);

		//uint8* fileBuffer = (uint8*)malloc(fileSize);

		//ionassertnd(file.Read(fileBuffer, fileSize));

		void* poolBlockPtr = nullptr;
		size_t poolBlockSize = 0;
		
		switch (type)
		{
			case EAssetType::Texture:
			{
				// @TODO: This will have to use an optimized texture format instead of plain pixel data

				Image image;
				image.Load(file);

				const uint8* pixelData = image.GetPixelData();

				poolBlockPtr = m_Owner->AllocateAssetData<EAssetType::Texture>(image.GetPixelDataSize());
				
				break;
			}
			default:
				// @TODO: Print thread id
				LOG_WARN("Unknown asset type in worker thread ({0})", 0);
		}

		refPtr->Data.Ptr = poolBlockPtr;
		refPtr->Data.Size = poolBlockSize;
	}
}
