#include "IonPCH.h"

#include "DebugProfiler.h"

namespace Ion
{
	// --------------------
	// Debug Timer --------

	DebugTimer::DebugTimer(std::string&& name, std::string&& type)
		: m_TimeInfo({ name, type })
	{ }

	void DebugTimer::Start()
	{
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		m_StartTime = startTime;
	}

	void DebugTimer::Stop()
	{
		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		m_TimeInfo.m_Time = (endTime - m_StartTime).count();
	}

	// -----------------------
	// Debug Profiler --------

	DebugProfiler* DebugProfiler::Get()
	{
		if (!s_Instance)
			s_Instance = new DebugProfiler;

		return s_Instance;
	}

	DebugTimer* DebugProfiler::RegisterTimer(std::string&& id, std::string&& name, std::string&& type)
	{
		if (!IsTimerRegistered(id))
		{
			DebugTimer* timer = new DebugTimer(std::move(name), std::move(type));
			Get()->m_Timers[id] = timer;
		}
		return Get()->m_Timers[id];
	}

	DebugTimer* DebugProfiler::FindTimer(const std::string& id)
	{
		auto info = Get()->m_Timers.find(id);
		if (info != Get()->m_Timers.end())
			return (*info).second;
		
		return nullptr;
	}

	bool DebugProfiler::IsTimerRegistered(const std::string& id)
	{
		return Get()->m_Timers.find(id) != Get()->m_Timers.end();
	}

	DebugProfiler* DebugProfiler::s_Instance = nullptr;

	// ---------------------
	// Scoped Timer --------

	ScopedTimer::ScopedTimer(std::string&& name, std::string&& type)
		: m_Name(name), m_Type(type), m_Timer(nullptr)
	{ }

	ScopedTimer::~ScopedTimer()
	{
		LOG_INFO("Stopped Timer [{0}]", m_Name);
		m_Timer->Stop();
	}

	void ScopedTimer::Register(std::string&& id)
	{
		m_Timer = DebugProfiler::RegisterTimer(std::move(id), std::move(m_Name), std::move(m_Type));
	}

	void ScopedTimer::Start()
	{
		LOG_INFO("Started Timer [{0}]", m_Name);
		m_Timer->Start();
	}
}
