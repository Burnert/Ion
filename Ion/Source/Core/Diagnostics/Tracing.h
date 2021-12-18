#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreMacros.h"

#if (ION_DEBUG || ION_RELEASE && ION_RELEASE_TRACING) && !ION_NO_TRACING || ION_FORCE_TRACING
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
		struct TraceStart
		{
			const char* Name;
			int64 Timestamp;
			int32 PID;
			int32 TID;
		};
		struct TraceResult
		{
			const char* Name;
			int64 Timestamp;
			int64 EndTimestamp;
			double Duration;
			int32 PID;
			int32 TID;
		};

		using TraceResultsArray = TArray<TraceResult>;
		using TraceStartMap = THashMap<String, TraceStart>;
		using NamedTraceResultsMap = THashMap<String, TraceResult*>;
		using ThreadNameCache = THashMap<int32, String>;

		class ION_API ScopedTracer
		{
		public:
			// @TODO: Make the name formattable
			ScopedTracer(const char* name);
			~ScopedTracer();

			int64 GetTimestamp();

		protected:
			void CacheStart();
			void CacheResult(int64 endTime, double duration);

			// Internal
			ScopedTracer();

		private:
			const char* m_Name;
			int64 m_StartTime;
			bool m_bRunning;
			bool m_bInternal;

			friend class DebugTracing;
		};

		static void Init();
		static void Shutdown();

		static void BeginSession(const char* name);
		static void EndSession();
		static bool HasSessionStarted();
		static const char* GetCurrentSessionName();

		static void StartSessionRecording();
		static void StopSessionRecording();
		static bool IsSessionRecording();

		static void DumpResults(bool bEndSession = false);

		static int64 TimestampToMicroseconds(int64 timestamp);

	private:
		static void WriteBeginEvent(const char* name, int64 timestamp, int32 pid, const char* threadDesc, String& outString);
		static void WriteEndEvent(int64 timestamp, int32 pid, const char* threadDesc, String& outString);

	private:
		static TraceResultsArray s_TraceResults;
		static TraceStartMap s_TraceStarts;
		static NamedTraceResultsMap s_NamedTraceResults;
		static ThreadNameCache s_ThreadNameCache;
		static TUnique<File> s_SessionDumpFile;
		static Mutex s_ResultsMutex;
		static Mutex s_SessionMutex;
		static const char* s_CurrentSessionName;
		static bool s_bSessionRecording;
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
