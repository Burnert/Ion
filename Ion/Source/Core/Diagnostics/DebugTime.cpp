#include "IonPCH.h"

#include "DebugTime.h"

namespace Ion
{
	DebugTimer::DebugTimer(bool bStart) :
		m_bRunning(false),
		m_StartTime(0),
		m_DurationNs(0)
	{
		if (bStart)
			Start();
	}

	DebugTimer::~DebugTimer()
	{
	}

	void DebugTimer::Start()
	{
		if (m_bRunning || m_DurationNs)
			return;

		m_bRunning = true;
		m_StartTime = GetPlatformTimestamp();
	}

	void DebugTimer::Stop()
	{
		if (!m_bRunning)
			return;

		m_bRunning = false;
		m_DurationNs = CalcPlatformDurationNs(m_StartTime, GetPlatformTimestamp());
	}

	static double ConvertNsToUnit(int64 ns, EDebugTimerTimeUnit unit)
	{
		switch (unit)
		{
			case EDebugTimerTimeUnit::Second:      return (double)ns * 0.000000001;
			case EDebugTimerTimeUnit::Millisecond: return (double)ns * 0.000001;
			case EDebugTimerTimeUnit::Microsecond: return (double)ns * 0.001;
			case EDebugTimerTimeUnit::Nanosecond:  return (double)ns;
		}
		return (double)ns;
	}

	double DebugTimer::GetTime(EDebugTimerTimeUnit unit)
	{
		return ConvertNsToUnit(GetTimeNs(), unit);
	}

	int64 DebugTimer::GetTimeNs()
	{
		if (!m_bRunning)
		{
			return m_DurationNs;
		}
		return CalcPlatformDurationNs(m_StartTime, GetPlatformTimestamp());
	}

	void DebugTimer::PrintTimer(const String& name, EDebugTimerTimeUnit unit)
	{
		LOG_INFO(unit == EDebugTimerTimeUnit::Nanosecond ? "[Timer] {0} = {1:.0f}{2}" : "[Timer] {0} = {1:.3f}{2}",
			name, GetTime(unit), DebugTimerTimeUnitToStringShort(unit));
	}
}
