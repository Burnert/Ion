#include "IonPCH.h"

#include "Tracing.h"

#if ION_ENABLE_TRACING

namespace Ion
{
	void DebugTracing::BeginSession(const char* name)
	{
		#pragma warning(disable:26451)
		#pragma warning(disable:6387)
		#pragma warning(disable:6255)

		int32 nameLength = (int32)strlen(name);
		ionassert(nameLength < 50);

		bool bResult;

		ionassert(!HasSessionStarted());
		{
			UniqueLock lockSession(s_SessionMutex);
			s_CurrentSessionName = name;
		}
		wchar* nameW = (wchar*)_alloca((nameLength + 1) * sizeof(wchar));
		StringConverter::CharToWChar(name, nameW, nameLength);

		std::filesystem::create_directory("Debug");

		// @TODO: Make a date reading utility and fill the Xs in the filename
		wchar filename[100];
		memset(filename, 0, sizeof(filename));
		swprintf_s(filename, L"Debug/Trace_XX-XX-XX_XX-XX-XXXX_%s.json", nameW);

		UniqueLock lockResults(s_ResultsMutex);
		s_SessionDumpFile = MakeUnique<File>(filename, EFileMode::Write | EFileMode::Reset | EFileMode::CreateNew);

		//bResult = s_SessionDumpFile->Open(IO::FM_Write | IO::FM_Reset);
		//ionassertnd(bResult, "Session dump file cannot be opened!");

		bResult = s_SessionDumpFile->Write("{\"traceEvents\":["); // Header
		ionassertnd(bResult, "Session dump file cannot be written!");
	}

	void DebugTracing::EndSession()
	{
		UniqueLock lockResults(s_ResultsMutex);
		ionassert(HasSessionStarted());
		// Go back one character, discarding the comma from the last event
		s_SessionDumpFile->AddOffset(-1);
		s_SessionDumpFile->Write("]}"); // Footer
		s_SessionDumpFile->Close();
		s_SessionDumpFile = nullptr;

		UniqueLock lockSession(s_SessionMutex);
		s_CurrentSessionName = nullptr;
	}

	bool DebugTracing::HasSessionStarted()
	{
		UniqueLock lock(s_SessionMutex);
		return (bool)s_CurrentSessionName;
	}

	const char* DebugTracing::GetCurrentSessionName()
	{
		UniqueLock lock(s_SessionMutex);
		return s_CurrentSessionName;
	}

	void DebugTracing::StartSessionRecording()
	{
		ionassert(HasSessionStarted());
		ionassert(!IsSessionRecording());

		UniqueLock lock(s_SessionMutex);
		s_bSessionRecording = true;
	}

	void DebugTracing::StopSessionRecording()
	{
		ionassert(HasSessionStarted());
		ionassert(IsSessionRecording());
		{
			UniqueLock lock(s_SessionMutex);
			s_bSessionRecording = false;
		}
		UniqueLock lock(s_ResultsMutex);
		DumpResults(true);
	}

	bool DebugTracing::IsSessionRecording()
	{
		UniqueLock lock(s_SessionMutex);
		return s_bSessionRecording;
	}

	// @TODO: Make this output the results in a separate thread

	void DebugTracing::DumpResults(bool bEndSession)
	{
		ionassert(HasSessionStarted());

		ScopedTracer dumpTracer;

		String fileDumpTemp;
		fileDumpTemp.reserve(ION_TRACE_DUMP_THRESHOLD * 100);

		TraceResultsArray localResults;
		ThreadNameCache localThreadNameCache;
		{
			//UniqueLock lock(s_ResultsMutex);
			s_TraceResults.swap(localResults);
			localThreadNameCache = s_ThreadNameCache;
		}

		for (const TraceResult& result : localResults)
		{
			WriteBeginEvent(result.Name, TimestampToMicroseconds(result.Timestamp),
				result.PID, localThreadNameCache.at(result.TID).c_str(), fileDumpTemp);

			WriteEndEvent(TimestampToMicroseconds(result.EndTimestamp),
				result.PID, localThreadNameCache.at(result.TID).c_str(), fileDumpTemp);
		}
		if (bEndSession)
		{
			TraceStartMap localTraceStarts;
			s_TraceStarts.swap(localTraceStarts);
			for (auto& [key, traceStart] : localTraceStarts)
			{
				WriteBeginEvent(traceStart.Name, TimestampToMicroseconds(traceStart.Timestamp),
					traceStart.PID, localThreadNameCache.at(traceStart.TID).c_str(), fileDumpTemp);
			}
		}
		// Write trace cache dump event
		{
			int32 pid = GetCurrentProcessId();
			int32 tid = GetCurrentThreadId();

			WriteBeginEvent("TRACE_CACHE_DUMP", TimestampToMicroseconds(dumpTracer.m_StartTime),
				pid, localThreadNameCache.at(tid).c_str(), fileDumpTemp);

			WriteEndEvent(TimestampToMicroseconds(dumpTracer.GetTimestamp()),
				pid, localThreadNameCache.at(tid).c_str(), fileDumpTemp);
		}

		s_SessionDumpFile->Write(fileDumpTemp.c_str(), fileDumpTemp.size());
	}

	void DebugTracing::ScopedTracer::CacheStart()
	{
		ionassert(HasSessionStarted());

		UniqueLock lock(s_ResultsMutex);

		// @TODO: Make this platform independent
		int32 pid = GetCurrentProcessId();
		int32 tid = GetCurrentThreadId();

		// Cache the debug thread name
		if (s_ThreadNameCache.find(tid) == s_ThreadNameCache.end())
		{
			wchar* desc;
			GetThreadDescription(GetCurrentThread(), &desc);
			wchar descStr[100];
			swprintf_s(descStr, L"%s (%d)", desc, tid);
			LocalFree(desc);
			s_ThreadNameCache.emplace(tid, StringConverter::WStringToString(descStr));
		}

		String key = m_Name + ToString(m_StartTime);
		s_TraceStarts.emplace(key, TraceStart { m_Name, m_StartTime, pid, tid });
	}

	void DebugTracing::ScopedTracer::CacheResult(int64 endTime, double duration)
	{
		ionassert(HasSessionStarted());

		UniqueLock lock(s_ResultsMutex);

		String key = m_Name + ToString(m_StartTime);
		s_TraceStarts.erase(key);

		// @TODO: Make this platform independent
		int32 pid = GetCurrentProcessId();
		int32 tid = GetCurrentThreadId();

		double durationMs = duration * 0.001;
		TraceResult& traceResult = s_TraceResults.emplace_back<TraceResult>({ m_Name, m_StartTime, endTime, duration, pid, tid });
		s_NamedTraceResults[m_Name] = &traceResult;

		if (s_TraceResults.size() >= ION_TRACE_DUMP_THRESHOLD)
		{
			s_NamedTraceResults.clear();
			// @TODO: Make this happen in another thread
			DumpResults();
		}
	}

	void DebugTracing::WriteBeginEvent(const char* name, int64 timestamp, int32 pid, const char* threadDesc, String& outString)
	{
		int32 charsWritten;
		// 4096 for some crazy template magic
		char eventBuffer[4096] = { 0 };
		// Begin event
		charsWritten = sprintf_s(eventBuffer, "{\"name\":\"%s\",\"cat\":\"%s\",\"ph\":\"B\",\"ts\":%lld,\"pid\":%d,\"tid\":\"%s\"},",
			name, "TODO CATEGORY", timestamp, pid, threadDesc);
		outString += eventBuffer;
	}

	void DebugTracing::WriteEndEvent(int64 timestamp, int32 pid, const char* threadDesc, String& outString)
	{
		int32 charsWritten;
		char eventBuffer[4096] = { 0 };
		// End event
		charsWritten = sprintf_s(eventBuffer, "{\"ph\":\"E\",\"ts\":%lld,\"pid\":%d,\"tid\":\"%s\"},",
			timestamp, pid, threadDesc);
		outString += eventBuffer;
	}

	DebugTracing::TraceResultsArray DebugTracing::s_TraceResults;
	DebugTracing::TraceStartMap DebugTracing::s_TraceStarts;
	DebugTracing::NamedTraceResultsMap DebugTracing::s_NamedTraceResults;
	DebugTracing::ThreadNameCache DebugTracing::s_ThreadNameCache;
	TUnique<File> DebugTracing::s_SessionDumpFile = nullptr;
	Mutex DebugTracing::s_ResultsMutex;
	Mutex DebugTracing::s_SessionMutex;
	const char* DebugTracing::s_CurrentSessionName = nullptr;
	bool DebugTracing::s_bSessionRecording = false;
}

#endif
