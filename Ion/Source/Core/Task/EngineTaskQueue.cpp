#include "IonPCH.h"

#include "EngineTaskQueue.h"

namespace Ion
{
	TUnique<TaskQueue> g_EngineTaskQueue;

	namespace EngineTaskQueue
	{
		void Schedule(FTaskWork& work)
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			g_EngineTaskQueue->Schedule(work);
		}

		void Schedule(const TShared<FTaskWork>& work)
		{
			ionassert(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			g_EngineTaskQueue->Schedule(work);
		}

		void Init()
		{
			ionassert(!g_EngineTaskQueue, "The Engine Task Queue has already been initialized.");
			g_EngineTaskQueue = MakeUnique<TaskQueue>();
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
			ionassertnd(g_EngineTaskQueue, "The Engine Task Queue has not been initialized yet.");
			return *g_EngineTaskQueue.get();
		}
	}
}
