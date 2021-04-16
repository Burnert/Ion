#include "IonPCH.h"

#include "Core/Diagnostics/Tracing.h"

#if ION_ENABLE_TRACING

namespace Ion
{
	static llong g_PerformanceFrequency;
	static llong g_InitTime;

	void DebugTracing::Init()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&g_PerformanceFrequency);
		QueryPerformanceCounter((LARGE_INTEGER*)&g_InitTime);

		s_SessionDumpFile = File::Create();
	}

	void DebugTracing::Shutdown()
	{
		s_SessionDumpFile->Close();
		delete s_SessionDumpFile;
	}

	llong DebugTracing::TimestampToMicroseconds(llong timestamp)
	{
		return (timestamp * 1000000) / g_PerformanceFrequency;
	}

	DebugTracing::ScopedTracer::ScopedTracer(const char* name) :
		m_Name(name),
		m_bRunning(true)
	{
		ionassert(HasSessionStarted());
		llong startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
		m_StartTime = startTime - g_InitTime;
	}

	DebugTracing::ScopedTracer::~ScopedTracer()
	{
		ionassert(HasSessionStarted());
		if (!m_bRunning)
		{
			return;
		}
		m_bRunning = false;

		llong endTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
		endTime -= g_InitTime;
		float duration = (float)((endTime - m_StartTime) * 1000000 / g_PerformanceFrequency);
		CacheResult(endTime, duration);
	}
}

#endif
