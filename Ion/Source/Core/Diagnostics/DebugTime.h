#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreApi.h"

namespace Ion
{
	enum class EDebugTimerTimeUnit
	{
		Second,
		Millisecond,
		Microsecond,
		Nanosecond,
	};

	class ION_API DebugTimer
	{
	public:
		DebugTimer(bool bStart = true);
		~DebugTimer();

		void Start();
		void Stop();

		double GetTime(EDebugTimerTimeUnit unit);
		int64 GetTimeNs();

		void PrintTimer(const String& name, EDebugTimerTimeUnit unit);

		inline static constexpr const char* DebugTimerTimeUnitToStringShort(EDebugTimerTimeUnit unit)
		{
			switch (unit)
			{
				case EDebugTimerTimeUnit::Second:      return "s";
				case EDebugTimerTimeUnit::Millisecond: return "ms";
				case EDebugTimerTimeUnit::Microsecond: return "us";
				case EDebugTimerTimeUnit::Nanosecond:  return "ns";
			}
			return "";
		}

		static void InitPlatform();

	private:
		static int64 GetPlatformTimestamp();
		static int64 CalcPlatformDurationNs(int64 ts1, int64 ts2);
		
	private:
		bool m_bRunning;
		int64 m_StartTime;
		int64 m_DurationNs;
	};
}
