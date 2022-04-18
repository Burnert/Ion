#include "IonPCH.h"

#include "TaskQueue.h"

namespace Ion
{
	// TaskWorker ---------------------------------------------------

	TaskWorker::TaskWorker() :
		m_bExit(false),
		m_Owner(nullptr)
	{
	}

	void TaskWorker::Start()
	{
		m_Thread = Thread(&TaskWorker::WorkerProc, this);
	}

	void TaskWorker::Exit()
	{
		m_bExit = true;
	}

	void TaskWorker::SetOwner(TaskQueue* owner)
	{
		ionassert(!m_Owner);
		m_Owner = owner;
	}

	void TaskWorker::WorkerProc()
	{
		ionassertnd(m_Owner);

		Platform::SetCurrentThreadDescription(L"AssetWorker");

		TQueue<TShared<FTaskWork>>& queue = m_Owner->m_WorkQueue;

		while (!m_bExit)
		{
			// Wait for the work
			{
				UniqueLock lock(m_Owner->m_WorkQueueMutex);

				m_Owner->m_WorkQueueWorkersCV.wait(lock, [this, &queue]
				{
					// Don't wait if there is any work available
					// or the threads needs to exit
					return queue.size() || m_bExit;
				});
				if (m_bExit)
					break;

				m_CurrentWork = queue.front();
				queue.pop();
			}

			m_CurrentWork->Execute(*m_Owner);
			m_CurrentWork.reset();
		}
	}

	// TaskQueue ---------------------------------------------------

	TaskQueue::TaskQueue() :
		TaskQueue(std::thread::hardware_concurrency())
	{
	}

	TaskQueue::TaskQueue(int32 nThreads) :
		m_Workers(nThreads)
	{
		for (TaskWorker& worker : m_Workers)
		{
			worker.m_Owner = this;
			worker.Start();
		}
	}

	void TaskQueue::Schedule(const TShared<FTaskWork>& work)
	{
		// Lock the queue, notify a free worker
		{
			UniqueLock lock(m_WorkQueueMutex);
			m_WorkQueue.emplace(work);
		}
		m_WorkQueueWorkersCV.notify_one();
	}

	void TaskQueue::DispatchMessages()
	{
		// Retrieve the messages and unlock the mutex right away
		TQueue<FTaskMessage> messages;
		{
			UniqueLock lock(m_MessageQueueMutex);
			messages.swap(m_MessageQueue);
		}

		while (!messages.empty())
		{
			FTaskMessage& message = messages.front();

			message.OnDispatch();

			messages.pop();
		}
	}

	void TaskQueue::Shutdown()
	{
		for (TaskWorker& worker : m_Workers)
		{
			worker.Exit();
		}
		// Make sure the free workers stop waiting
		m_WorkQueueWorkersCV.notify_all();

		for (TaskWorker& worker : m_Workers)
		{
			if (worker.m_Thread.joinable())
				worker.m_Thread.join();
		}

		m_Workers.clear();
	}

	void TaskQueue::PushMessage(FTaskMessage& message)
	{
		UniqueLock lock(m_MessageQueueMutex);
		m_MessageQueue.push(message);
	}

	TaskQueue::~TaskQueue()
	{
		// Shutdown if it hasn't been called yet.
		if (!m_Workers.empty())
		{
			Shutdown();
		}
	}
}
