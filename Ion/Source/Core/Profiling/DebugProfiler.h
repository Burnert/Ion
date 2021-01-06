#pragma once

#include "Core/CoreApi.h"

#define SCOPED_TIMER(id, name, type) \
ScopedTimer id = ScopedTimer(name, type); \
id.Register(#id); \
id.Start()

namespace Ion
{
	struct DebugTimeInfo
	{
		friend class DebugTimer;

		std::string Name;
		std::string Type;

		DebugTimeInfo(const std::string& name, const std::string& type)
			: Name(name), Type(type), m_Time(0) { }

		FORCEINLINE uint   GetTimeMs() const { return (uint)(m_Time / 1000000u); }
		FORCEINLINE ullong GetTimeUs() const { return (uint)(m_Time / 1000u); }
		FORCEINLINE ullong GetTimeNs() const { return m_Time; }

	private:
		ullong m_Time = 0;
	};

	/* Timer initialisation and maintenance should be performed by a helper class. */
	class DebugTimer
	{
		friend class DebugProfiler;
		friend class ScopedTimer;
	public:
		FORCEINLINE DebugTimeInfo GetTimeInfo() const { return m_TimeInfo; }

	private:
		DebugTimer(std::string&& name, std::string&& type);

		void Start();
		void Stop();

		DebugTimeInfo m_TimeInfo;
		std::chrono::steady_clock::time_point m_StartTime;
	};

	class ION_API DebugProfiler
	{
	public:
		static DebugProfiler* Get();

		static DebugTimer* RegisterTimer(std::string&& id, std::string&& name, std::string&& type);
		static DebugTimer* FindTimer(const std::string& id);
		static bool IsTimerRegistered(const std::string& id);

	private:
		static DebugProfiler* s_Instance;

		std::unordered_map<std::string, DebugTimer*> m_Timers;
	};

	class ION_API ScopedTimer
	{
	public:
		ScopedTimer(std::string&& name, std::string&& type = "Generic");
		~ScopedTimer();

		void Register(std::string&& id);

		void Start();

	private:
		DebugTimer* m_Timer;

		std::string m_Name;
		std::string m_Type;
	};
}
