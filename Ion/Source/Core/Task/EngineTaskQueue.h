#pragma once

#include "TaskQueue.h"

namespace Ion
{
	/**
	 * @brief Main Engine Task Queue
	 * 
	 * @details Use the members of the EngineTaskQueue namespace to access it.
	 */
	extern std::unique_ptr<TaskQueue> g_EngineTaskQueue;

	/**
	 * @brief Engine Task Queue object wrapper functions
	 * 
	 * @details The Engine Task Queue is initialized on application
	 * start (before client OnInit method is called). Don't try to
	 * access it earlier.
	 * 
	 * @see TaskQueue
	 * @see g_EngineTaskQueue
	 */
	namespace EngineTaskQueue
	{
		/**
		 * @brief Add a work to the Engine Task Queue, which will be executed
		 * by a free worker thread.
		 * 
		 * @see TaskQueue::Schedule(T& work)
		 *
		 * @tparam T must inherit from FTaskWork
		 * @param work Work object
		 */
		template<typename T>
		void Schedule(T& work);

		/**
		 * @brief Same as Schedule(FTaskWork& work), but uses
		 * an existing shader pointer.
		 * 
		 * @see TaskQueue::Schedule(const std::shared_ptr<FTaskWork>& work)
		 * @see Schedule(FTaskWork& work)
		 *
		 * @param work Work pointer
		 */
		void Schedule(const std::shared_ptr<FTaskWork>& work);

		/**
		 * @brief Creates the Engine Task Queue object, which will then be
		 * available in the g_EngineTaskQueue global variable.
		 * 
		 * @details The Task Queue will have maximum number of workers.
		 * (std::thread::hardware_concurrency())
		 * Functions in this namespace should be used instead of
		 * accessing the g_EngineTaskQueue variable directly.
		 * 
		 * @see TaskQueue::TaskQueue()
		 */
		void Init();

		/**
		 * @brief Ends the execution of the Engine Task Queue worker threads.
		 * Call this before the application exits
		 * 
		 * @see TaskQueue::Shutdown()
		 */
		void Shutdown();

		/**
		 * @brief Dispach all the messages, which are currently in
		 * the message queue. This should be called at the beginning
		 * of each frame.
		 * 
		 * @see TaskQueue::DispatchMessages()
		 */
		void Update();

		/**
		 * @brief Get the Engine Task Queue object reference
		 * 
		 * @return TaskQueue reference
		 */
		TaskQueue& Get();

		template<typename T>
		inline void Schedule(T& work)
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			g_EngineTaskQueue->Schedule(work);
		}
	}
}
