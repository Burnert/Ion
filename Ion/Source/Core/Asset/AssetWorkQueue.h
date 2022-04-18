#pragma once

#include "AssetCommon.h"

#define DEFAULT_ASSET_WORKER_COUNT 4

namespace Ion
{
	struct IAssetMessageQueueProvider
	{
		/**
		 * @brief Call this in an overloaded IAssetWork::Execute method
		 * from a custom Asset Work type to post a message, that will get
		 * executed by the main thread at the beginning of the next frame.
		 * 
		 * @see IAssetWork::Execute(IMessageQueueProvider& messageQueue)
		 * 
		 * @param message Message with an OnDispatch function
		 */
		virtual void PushMessage(AssetMessage& message) = 0;
	};

	enum class EAssetWork : uint8
	{
		Null = 0,
		Load,
	};

	struct IAssetWork
	{
		virtual void Execute(IAssetMessageQueueProvider& messageQueue) const = 0;
		virtual EAssetWork GetType() const = 0;
	};

#define ASSET_WORK_TYPE(type) \
EAssetWork Type = type; \
virtual EAssetWork GetType() const override \
{ \
	return Type; \
}

	struct AssetLoadWork : IAssetWork
	{
		AssetLoadWork(const FilePath& assetPath);

		TFunction<void(const AssetData&)> OnLoad;
		TFunction<void(/*ErrorDesc*/)> OnError;

		/**
		 * @brief Loads the asset from the file specified on construction.
		 * 
		 * @details This executes on a worker thread, so it cannot interact
		 * with the other parts of the program apart from the message queue.
		 * 
		 * @param messageQueue Queue provider to push the messages to (OnLoad, OnError, etc.).
		 */
		virtual void Execute(IAssetMessageQueueProvider& messageQueue) const override;

		ASSET_WORK_TYPE(EAssetWork::Load)

	private:
		FilePath m_AssetPath;
	};

	struct AssetMessage
	{
		AssetMessage(const TFunction<void()>& onDispatch) :
			OnDispatch(onDispatch)
		{
		}

		TFunction<void()> OnDispatch;
	};

	class ION_API AssetWorker
	{
	public:
		AssetWorker();

	private:
		void Start();
		void Exit();

		void SetOwner(AssetWorkQueue* owner);

		// Worker Thread Functions ------------------------------------------------

		void WorkerProc();

	private:
		Thread m_Thread;
		TShared<IAssetWork> m_CurrentWork;
		AssetWorkQueue* m_Owner;

		bool m_bActive;

		friend class AssetWorkQueue;
	};

	class ION_API AssetWorkQueue : public IAssetMessageQueueProvider
	{
	public:
		AssetWorkQueue();

		/**
		 * @brief Add a work to the queue, which will be executed
		 * by a free worker thread.
		 * 
		 * @tparam T Work type - must inherit from IAssetWork
		 * @param work Work object
		 */
		template<typename T>
		void Schedule(T& work);

		/**
		 * @brief Dispach all the messages, which currently are in
		 * the message queue. This should be called at the beginning
		 * of each frame.
		 */
		void DispatchMessages();

		/**
		 * @brief Ends the execution of the worker threads.
		 * Call this before the application exits
		 */
		void Shutdown();

		// IMessageQueueProvider overrides:

		/**
		 * @brief Called by IAssetWork::Execute.
		 * Adds a message to the message queue.
		 * 
		 * @see IAssetWork::Execute(IMessageQueueProvider& messageQueue)
		 * 
		 * @param message Message object
		 */
		virtual void PushMessage(AssetMessage& message) override;

		// End of IMessageQueueProvider overrides

	private:
		TArray<AssetWorker> m_Workers;

		TQueue<TShared<IAssetWork>> m_WorkQueue;
		Mutex m_WorkQueueMutex;
		ConditionVariable m_WorkQueueWorkersCV;

		TQueue<AssetMessage> m_MessageQueue;
		Mutex m_MessageQueueMutex;
		ConditionVariable m_MessageQueueCV;

		friend class AssetWorker;
	};

	template<typename T>
	inline void AssetWorkQueue::Schedule(T& work)
	{
		static_assert(TIsBaseOfV<IAssetWork, T>);

		// Lock the queue, notify a free worker
		{
			UniqueLock lock(m_WorkQueueMutex);
			m_WorkQueue.emplace(MakeShared<T>(Move(work)));
		}
		m_WorkQueueWorkersCV.notify_one();
	}
}
