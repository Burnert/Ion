#include "IonPCH.h"

#include "Core/Diagnostics/DebugTime.h"

namespace Ion
{
	static int64 g_PerformanceFrequency = 0;
	static int64 g_InitTime;

	void DebugTimer::InitPlatform()
	{
		ionassert(!g_PerformanceFrequency);

		QueryPerformanceFrequency((LARGE_INTEGER*)&g_PerformanceFrequency);
		QueryPerformanceCounter((LARGE_INTEGER*)&g_InitTime);
	}

	int64 DebugTimer::GetPlatformTimestamp()
	{
		int64 timestamp;
		QueryPerformanceCounter((LARGE_INTEGER*)&timestamp);
		return timestamp - g_InitTime;
	}

	int64 DebugTimer::CalcPlatformDurationNs(int64 ts1, int64 ts2)
	{
		return (int64)((double)(ts2 - ts1) / (double)g_PerformanceFrequency * 1000000000.0);
	}
}
