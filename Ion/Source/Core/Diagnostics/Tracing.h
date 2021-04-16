#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreMacros.h"

#if ION_DEBUG && !ION_NO_TRACING || ION_FORCE_TRACING
#define ION_ENABLE_TRACING 1
#else
#define ION_ENABLE_TRACING 0
#endif

#if ION_ENABLE_TRACING

namespace Ion
{
	class ION_API DebugTracing
	{
	public:
		struct TraceResult
		{
			const char* Name;
			llong Timestamp;
			llong EndTimestamp;
			float Duration;
		};

		class ION_API ScopedTracer
		{
		public:
			ScopedTracer(const char* name);
			~ScopedTracer();

		protected:
			void CacheResult(llong endTime, float duration);

		private:
			const char* m_Name;
			llong m_StartTime;
			bool m_bRunning;
		};

		static void Init();
		static void Shutdown();

		static void BeginSession(const char* name);
		static void EndSession();
		inline static bool HasSessionStarted() { return (bool)s_CurrentSessionName; }
		inline static const char* GetCurrentSessionName() { return s_CurrentSessionName; }

		static void StartSessionRecording();
		static void StopSessionRecording();
		inline static bool IsSessionRecording() { return s_bSessionRecording; }

		static void DumpResults();

		static llong TimestampToMicroseconds(llong timestamp);

	private:
		static const char* s_CurrentSessionName;
		static bool s_bSessionRecording;
		static std::vector<TraceResult> s_TraceResults;
		static std::unordered_map<String, TraceResult*> s_NamedTraceResults;
		static File* s_SessionDumpFile;
	};
}

#define TRACE_FUNCTION()            Ion::DebugTracing::ScopedTracer CAT(tracer_, __LINE__)(__FUNCSIG__)
#define TRACE_SCOPE(name)           Ion::DebugTracing::ScopedTracer CAT(tracer_, __LINE__)(name)
#define TRACE_BEGIN(localid, name)  Ion::DebugTracing::ScopedTracer CAT(tracer__, localid)(name)
#define TRACE_END(localid)          CAT(tracer__, localid).~ScopedTracer()

#define TRACE_SESSION_BEGIN(name)   Ion::DebugTracing::BeginSession(name)
#define TRACE_SESSION_END()         Ion::DebugTracing::EndSession()

#define TRACE_RECORD_START()        Ion::DebugTracing::StartSessionRecording()
#define TRACE_RECORD_STOP()         Ion::DebugTracing::StopSessionRecording()

#else

#define TRACE_FUNCTION()
#define TRACE_SCOPE(name)
#define TRACE_BEGIN(localid, name)
#define TRACE_END(localid)

#define TRACE_SESSION_BEGIN(name)
#define TRACE_SESSION_END()

#define TRACE_RECORD_START()
#define TRACE_RECORD_STOP()

#endif
