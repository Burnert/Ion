#include "Core/CorePCH.h"

#include "EngineTaskQueue.h"
#include "Core/Error/Error.h"

namespace Ion
{
	std::unique_ptr<TaskQueue> g_EngineTaskQueue;

	namespace EngineTaskQueue
	{
		void Schedule(const std::shared_ptr<FTaskWork>& work)
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			g_EngineTaskQueue->Schedule(work);
		}

		void Init()
		{
			ionassert(!g_EngineTaskQueue, "The Engine Task Queue has already been initialized.");
			g_EngineTaskQueue = std::make_unique<TaskQueue>();
		}

		void Shutdown()
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			g_EngineTaskQueue->Shutdown();
		}

		void Update()
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			g_EngineTaskQueue->DispatchMessages();
		}

		TaskQueue& Get()
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			return *g_EngineTaskQueue.get();
		}
	}
}
