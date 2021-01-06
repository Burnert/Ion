#pragma once

#include "Core/CoreApi.h"

/* Used to declare and register a debug performance counter with specified type */
#define DECLARE_PERFORMANCE_COUNTER(id, name, type) \
Ion::Performance::DebugCounter* DebugPerformance_##id = Ion::Performance::DebugProfiler::RegisterCounter(#id, name, type)

/* Used to declare and register a debug performance counter with a generic type */
#define DECLARE_PERFORMANCE_COUNTER_GENERIC(id, name) \
Ion::Performance::DebugCounter* DebugPerformance_##id = Ion::Performance::DebugProfiler::RegisterCounter(#id, name)

/* Creates and starts a scoped timer using an already declared performance counter.
   Measures the time starting from its location to the end of the scope it is in. */
#define SCOPED_PERFORMANCE_COUNTER(id) \
Ion::Performance::ScopedCounter id = Ion::Performance::ScopedCounter(DebugPerformance_##id)

/* Creates a manual timer using an already declared performance counter.
   Measures the time starting from where Start() method was called to where Stop() method was. */
#define MANUAL_PERFORMANCE_COUNTER(id) \
std::shared_ptr<Ion::Performance::ManualCounter> id = std::make_shared<Ion::Performance::ManualCounter>(DebugPerformance_##id)

/* Retrieves PerformanceCounterData struct from a declared timer. */
#define COUNTER_TIME_INFO(varName, counterId) \
Ion::Performance::PerformanceCounterData varName = Ion::Performance::DebugProfiler::FindCounter(counterId)->GetData()

namespace Ion
{
namespace Performance
{
	struct PerformanceCounterData
	{
		friend class DebugCounter;

		std::string Name;
		std::string Type;

		PerformanceCounterData(const std::string& name, const std::string& type)
			: Name(name), Type(type), m_Time(0) { }

		FORCEINLINE uint   GetTimeMs() const { return (uint)(m_Time / 1000000u); }
		FORCEINLINE ullong GetTimeUs() const { return (uint)(m_Time / 1000u); }
		FORCEINLINE ullong GetTimeNs() const { return m_Time; }

	private:
		ullong m_Time = 0;
	};

	/* Timer initialisation and maintenance should be performed by a helper class. */
	class DebugCounter
	{
		friend class DebugProfiler;
		friend class ScopedCounter;
		friend class ManualCounter;

	public:
		FORCEINLINE PerformanceCounterData GetData() const { return m_CounterData; }

	private:
		DebugCounter(std::string&& name, std::string&& type);

		void Start();
		void Stop();

		PerformanceCounterData m_CounterData;
		std::chrono::steady_clock::time_point m_StartTime;
		std::chrono::steady_clock::time_point m_EndTime;
	};

	class ION_API DebugProfiler
	{
	public:
		static DebugCounter* RegisterCounter(std::string&& id, std::string&& name, std::string&& type = "Generic");
		static DebugCounter* FindCounter(const std::string& id);
		static bool IsCounterRegistered(const std::string& id);

		static DebugProfiler* Get();

	private:
		static DebugProfiler* s_Instance;
		std::unordered_map<std::string, DebugCounter*> m_RegisteredCounters;
	};

	/* Use the SCOPED_PERFORMANCE_COUNTER macro to use this counter */
	class ION_API ScopedCounter
	{
	public:
		ScopedCounter();
		ScopedCounter(DebugCounter* counterHandle);
		~ScopedCounter();

		void Assign(DebugCounter* counterHandle);

	private:
		DebugCounter* m_CounterHandle;
	};

	/* Use the MANUAL_PERFORMANCE_COUNTER macro to use this counter */
	class ION_API ManualCounter
	{
	public:
		ManualCounter();
		ManualCounter(DebugCounter* counterHandle);

		void Assign(DebugCounter* counterHandle);

		void Start();
		void Stop();

	private:
		DebugCounter* m_CounterHandle;
	};
}
}
