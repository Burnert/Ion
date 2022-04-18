#pragma once

#include "TaskFwd.h"

namespace Ion
{
	struct IMessageQueueProvider
	{
		/**
		 * @brief Call this in an Execute lambda passed to the
		 * FTaskWork constructor to post a message, that will get
		 * executed by the main thread at the beginning of the next frame.
		 *
		 * @see FTaskWork::FTaskWork(const TFuncWorkExecute& execute)
		 * @see FTaskWork::Execute
		 *
		 * @param message Message with an OnDispatch function
		 */
		virtual void PushMessage(FTaskMessage& message) = 0;
	};

	using TFuncWorkExecute = TFunction<void(IMessageQueueProvider&)>;

	/**
	 * @brief Execute Functor wrapper that can be inserted into the work queue
	 */
	struct FTaskWork
	{
		/**
		 * @brief Implicit constructor from the lambda of type TFuncWorkExecute
		 * 
		 * @param execute Function to execute on the worker thread
		 */
		FTaskWork(const TFuncWorkExecute& execute) :
			Execute(execute)
		{
		}

		/**
		 * @brief Called by TaskWorker on the worker thread.
		 * 
		 * @see TaskWorker::WorkerProc()
		 */
		TFuncWorkExecute Execute;
	};

	using TFuncMessageOnDispatch = TFunction<void()>;

	/**
	 * @brief OnDispatch Functor wrapper that can be inserted into the message queue
	 */
	struct FTaskMessage
	{
		/**
		 * @brief Implicit constructor from the lambda of type TFuncMessageOnDispatch
		 * 
		 * @param onDispatch Function to execute in DispatchMessages
		 * on the main thread.
		 * 
		 * @see TaskQueue::DispatchMessages()
		 */
		FTaskMessage(const TFuncMessageOnDispatch& onDispatch) :
			OnDispatch(onDispatch)
		{
		}

		/**
		 * @brief Called by TaskQueue on the main thread.
		 * 
		 * @see TaskQueue::DispatchMessages()
		 */
		TFuncMessageOnDispatch OnDispatch;
	};

	class ION_API TaskWorker
	{
	public:
		TaskWorker();

	private:
		void Start();
		void Exit();

		void SetOwner(TaskQueue* owner);

		// Worker Thread Functions ------------------------------------------------

		void WorkerProc();

	private:
		Thread m_Thread;
		TShared<FTaskWork> m_CurrentWork;
		TaskQueue* m_Owner;

		bool m_bExit;

		friend class TaskQueue;
	};

	class ION_API TaskQueue : public IMessageQueueProvider
	{
	public:
		/**
		 * @brief Construct a new Task Queue object
		 * with maximum number of workers
		 * (std::thread::hardware_concurrency())
		 * 
		 */
		TaskQueue();

		/**
		 * @brief Construct a new Task Queue object
		 * with nThreads workers
		 * 
		 * @param nThreads Number of workers
		 */
		TaskQueue(int32 nThreads);

		/**
		 * @brief Add a work to the queue, which will be executed
		 * by a free worker thread.
		 * 
		 * @details The work object (work param) will get moved to
		 * the queue. Don't modify its contents after a call to
		 * this function.
		 *
		 * @param work Work object
		 */
		void Schedule(FTaskWork& work);

		/**
		 * @brief Same as Schedule(FTaskWork& work), but uses
		 * an existing shader pointer.
		 * 
		 * @param work Work pointer
		 * 
		 * @see Schedule(FTaskWork& work)
		 */
		void Schedule(const TShared<FTaskWork>& work);

		/**
		 * @brief Dispach all the messages, which are currently in
		 * the message queue. This should be called at the beginning
		 * of each frame.
		 * 
		 * @details If none of the FTaskWork functors, that will be used
		 * in this Task Queue, need to notify the main thread about
		 * an event (they don't send any messages), this function
		 * does not have to be ever called.
		 */
		void DispatchMessages();

		/**
		 * @brief Ends the execution of the worker threads.
		 * Call this before the application exits
		 */
		void Shutdown();

		// IMessageQueueProvider overrides:

		/**
		 * @brief Called by FTaskWork::Execute.
		 * Adds a message to the message queue.
		 *
		 * @see FTaskWork::Execute
		 *
		 * @param message Message object
		 */
		virtual void PushMessage(FTaskMessage& message) override;

		// End of IMessageQueueProvider overrides

	private:
		TArray<TaskWorker> m_Workers;

		TQueue<TShared<FTaskWork>> m_WorkQueue;
		Mutex m_WorkQueueMutex;
		ConditionVariable m_WorkQueueWorkersCV;

		TQueue<FTaskMessage> m_MessageQueue;
		Mutex m_MessageQueueMutex;
		ConditionVariable m_MessageQueueCV;

		friend class TaskWorker;
	};
}
