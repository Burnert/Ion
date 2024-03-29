#include "Core/CorePCH.h"

#include "DebugProfiler.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
namespace Performance
{
	// ----------------------------
	// Performance Counter --------

	DebugCounter::DebugCounter(String&& name, String&& type)
		: m_CounterData({ name, type }), m_Log(false)
	{ }

	void DebugCounter::Start()
	{
		if (m_Log)
			CoreLogger.Trace("Started Counter [{0}]", m_CounterData.Name);

		m_StartTime = std::chrono::steady_clock::now();
	}

	void DebugCounter::Stop()
	{
		if (m_Log)
			CoreLogger.Trace("Stopped Counter [{0}]", m_CounterData.Name);

		m_EndTime = std::chrono::steady_clock::now();
		m_CounterData.m_Time = (m_EndTime - m_StartTime).count();
	}

	// -----------------------
	// Debug Profiler --------

	DebugProfiler* DebugProfiler::s_Instance = nullptr;

	DebugProfiler* DebugProfiler::Get()
	{
		if (!s_Instance)
			s_Instance = new DebugProfiler;

		return s_Instance;
	}

	DebugCounter* DebugProfiler::RegisterCounter(String&& id, String&& name, String&& type)
	{
		DebugProfiler* instance = Get();
		if (!IsCounterRegistered(id))
		{
			DebugCounter* timer = new DebugCounter(std::move(name), std::move(type));
			instance->m_RegisteredCounters[id] = timer;
		}
		return instance->m_RegisteredCounters[id];
	}

	DebugCounter* DebugProfiler::FindCounter(const String& id)
	{
		DebugProfiler* instance = Get();
		auto info = instance->m_RegisteredCounters.find(id);
		if (info != instance->m_RegisteredCounters.end())
			return (*info).second;
		
		return nullptr;
	}

	bool DebugProfiler::IsCounterRegistered(const String& id)
	{
		DebugProfiler* instance = Get();
		return instance->m_RegisteredCounters.find(id) != instance->m_RegisteredCounters.end();
	}

	// -----------------------
	// Scoped Counter --------

	ScopedCounter::ScopedCounter()
		: ScopedCounter(nullptr)
	{ }

	ScopedCounter::ScopedCounter(DebugCounter* counterHandle)
		: m_CounterHandle(counterHandle)
	{
		m_CounterHandle->Start();
	}

	ScopedCounter::~ScopedCounter()
	{
		m_CounterHandle->Stop();
	}

	void ScopedCounter::Assign(DebugCounter* counterHandle)
	{
		m_CounterHandle = counterHandle;
	}

	// -----------------------
	// Manual Counter --------

	ManualCounter::ManualCounter()
		: ManualCounter(nullptr)
	{ }

	ManualCounter::ManualCounter(DebugCounter* counterHandle)
		: m_CounterHandle(counterHandle)
	{ }

	void ManualCounter::Assign(DebugCounter* counterHandle)
	{
		m_CounterHandle = counterHandle;
	}

	void ManualCounter::Start()
	{
		m_CounterHandle->Start();
	}

	void ManualCounter::Stop()
	{
		m_CounterHandle->Stop();
	}
}
}
