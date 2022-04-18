#include "IonPCH.h"

#include "AssetWorkQueue.h"

namespace Ion
{
	// AssetLoadWork -----------------------------------------------

	AssetLoadWork::AssetLoadWork(const FilePath& assetPath) :
		m_AssetPath(assetPath)
	{
	}

	void AssetLoadWork::Execute(IAssetMessageQueueProvider& messageQueue) const
	{
		ionassert(OnLoad);
		ionassert(OnError);

		// @TODO: Fix, temporary memory leak
		char* path = new char[200];
		strcpy_s(path, 200, StringConverter::WStringToString(m_AssetPath.ToString()).c_str());

		// @TODO: Load
		AssetData data;
		data.Data = path;
		data.Size = 200;

		std::this_thread::sleep_for(std::chrono::seconds(2));

		AssetMessage message = [OnLoad = this->OnLoad, data]
		{
			LOG_INFO("Hello from the main thread!");
			OnLoad(data);
		};
		messageQueue.PushMessage(message);
	}

	// AssetWorker -----------------------------------------------

	AssetWorker::AssetWorker() :
		m_bActive(true),
		m_Owner(nullptr)
	{
	}

	void AssetWorker::SetOwner(AssetWorkQueue* owner)
	{
		m_Owner = owner;
	}

	void AssetWorker::Start()
	{
		m_Thread = Thread(&AssetWorker::WorkerProc, this);
	}

	void AssetWorker::Exit()
	{
		m_bActive = false;
	}

	void AssetWorker::WorkerProc()
	{
		TRACE_FUNCTION();

		ionassertnd(m_Owner);

		Platform::SetCurrentThreadDescription(L"AssetWorker");

		TQueue<TShared<IAssetWork>>& queue = m_Owner->m_WorkQueue;

		while (m_bActive)
		{
			// Wait for the work
			{
				UniqueLock lock(m_Owner->m_WorkQueueMutex);

				m_Owner->m_WorkQueueWorkersCV.wait(lock, [&queue]
				{
					// Don't wait if there is any work available
					return queue.size();
				});

				m_CurrentWork = queue.front();
				queue.pop();
			}

			m_CurrentWork->Execute(*m_Owner);
			m_CurrentWork.reset();
		}
	}

	// AssetWorkQueue -----------------------------------------------

	AssetWorkQueue::AssetWorkQueue() :
		m_Workers(DEFAULT_ASSET_WORKER_COUNT)
	{
		for (AssetWorker& worker : m_Workers)
		{
			worker.m_Owner = this;
			worker.Start();
		}
	}

	void AssetWorkQueue::DispatchMessages()
	{
		// Retrieve the messages and unlock the mutex right away
		TQueue<AssetMessage> messages;
		{
			UniqueLock lock(m_MessageQueueMutex);
			messages.swap(m_MessageQueue);
		}

		while (!messages.empty())
		{
			AssetMessage& message = messages.front();

			message.OnDispatch();

			messages.pop();
		}
	}

	void AssetWorkQueue::Shutdown()
	{
		for (AssetWorker& worker : m_Workers)
		{
			worker.Exit();
		}

		for (AssetWorker& worker : m_Workers)
		{
			if (worker.m_Thread.joinable())
				worker.m_Thread.join();
		}
	}

	void AssetWorkQueue::PushMessage(AssetMessage& message)
	{
		UniqueLock lock(m_MessageQueueMutex);
		m_MessageQueue.push(message);
	}
}
