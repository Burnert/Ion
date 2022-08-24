#include "Core/CorePCH.h"

#include "Task.h"

namespace Ion
{
	AsyncTask::AsyncTask(const TFuncAsyncTaskOnExecute& onExecute)
	{
		m_Work = std::make_shared<FTaskWork>(onExecute);
	}

	void AsyncTask::Schedule(TaskQueue& taskQueue)
	{
		taskQueue.Schedule(m_Work);
	}

	void AsyncTask::Schedule()
	{
		EngineTaskQueue::Schedule(m_Work);
	}
}
