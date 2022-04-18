#pragma once

#include "EngineTaskQueue.h"

namespace Ion
{
	using TFuncAsyncTaskOnExecute = TFuncWorkExecute;

	/**
	 * @brief A class for creating Async Tasks that
	 * can be scheduled multiple times.
	 */
	class ION_API AsyncTask
	{
	public:
		AsyncTask(const TFuncAsyncTaskOnExecute& onExecute);

		/**
		 * @brief Add the work owned by this AsyncTask object
		 * to the specified task queue.
		 * 
		 * @param taskQueue TaskQueue to add the task to
		 */
		void Schedule(TaskQueue& taskQueue);

		/**
		 * @brief Add the work owned by this AsyncTask object
		 * to the engine task queue.
		 */
		void Schedule();

	private:
		TShared<FTaskWork> m_Work;
	};
}
