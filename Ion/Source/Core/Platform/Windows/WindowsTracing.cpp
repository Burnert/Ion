#include "IonPCH.h"

#include "Core/Diagnostics/Tracing.h"

#if ION_ENABLE_TRACING

namespace Ion
{
	static int64 g_PerformanceFrequency;
	static int64 g_InitTime;

	void DebugTracing::Init()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&g_PerformanceFrequency);
		QueryPerformanceCounter((LARGE_INTEGER*)&g_InitTime);
	}

	void DebugTracing::Shutdown()
	{ // @TODO: worker.join() here
	}

	int64 DebugTracing::TimestampToMicroseconds(int64 timestamp)
	{
		return (int64)(((double)(timestamp * 1000000)) / (double)g_PerformanceFrequency);
	}

	DebugTracing::ScopedTracer::ScopedTracer(const char* name) :
		m_Name(name),
		m_bRunning(IsSessionRecording()),
		m_StartTime(-1),
		m_bInternal(false)
	{
		if (!HasSessionStarted())
			return;

		if (m_bRunning)
		{
			m_StartTime = GetTimestamp();

			CacheStart();
		}
	}

	DebugTracing::ScopedTracer::~ScopedTracer()
	{
		if (m_bInternal)
			return;

		if (!HasSessionStarted())
			return;

		// If m_StartTime is -1 it means that recording was started after the ScopedTracer was created
		if (!m_bRunning || m_StartTime == -1)
		{
			return;
		}
		m_bRunning = false;

		int64 endTime = GetTimestamp();
		double duration = ((double)((endTime - m_StartTime) * 1000000)) / (double)g_PerformanceFrequency;

		CacheResult(endTime, duration);
	}

	int64 DebugTracing::ScopedTracer::GetTimestamp()
	{
		int64 time;
		QueryPerformanceCounter((LARGE_INTEGER*)&time);
		return time - g_InitTime;
	}

	DebugTracing::ScopedTracer::ScopedTracer() :
		m_Name(nullptr),
		m_bRunning(true),
		m_StartTime(-1),
		m_bInternal(true)
	{
		if (!HasSessionStarted())
			return;

		m_StartTime = GetTimestamp();
	}
}

#endif
