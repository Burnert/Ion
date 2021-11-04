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
		s_CurrentSessionName = name;

		wchar* nameW = (wchar*)_alloca((nameLength + 1) * sizeof(wchar));
		StringConverter::CharToWChar(name, nameW, nameLength);

		std::filesystem::create_directory("Debug");

		// @TODO: Make a date reading utility and fill the Xs in the filename
		wchar filename[100];
		memset(filename, 0, sizeof(filename));
		swprintf_s(filename, L"Debug/Trace_XX-XX-XX_XX-XX-XXXX_%s.json", nameW);

		s_SessionDumpFile->SetFilename(filename);

		bResult = s_SessionDumpFile->Open(IO::FM_Write | IO::FM_Reset);
		ionassertnd(bResult, "Session dump file cannot be opened!");

		bResult = s_SessionDumpFile->Write("{\"traceEvents\":["); // Header
		ionassertnd(bResult, "Session dump file cannot be written!");
	}

	void DebugTracing::EndSession()
	{
		ionassert(HasSessionStarted());
		// Go back one character, discarding the comma from the last event
		s_SessionDumpFile->AddOffset(-1);
		s_SessionDumpFile->Write("]}"); // Footer
		s_SessionDumpFile->Close();

		s_CurrentSessionName = nullptr;
	}

	void DebugTracing::StartSessionRecording()
	{
		ionassert(HasSessionStarted());
		ionassert(!IsSessionRecording());
		s_bSessionRecording = true;
	}

	void DebugTracing::StopSessionRecording()
	{
		ionassert(HasSessionStarted());
		ionassert(IsSessionRecording());
		s_bSessionRecording = false;
		DumpResults();
	}

	void DebugTracing::DumpResults()
	{
		ionassert(HasSessionStarted());

		for (const TraceResult& result : s_TraceResults)
		{
			int32 charsWritten;
			// 4096 for some crazy template magic
			char eventBuffer[4096];
			memset(eventBuffer, 0, 4096);
			// Begin event
			charsWritten = sprintf_s(eventBuffer, "{\"name\":\"%s\",\"cat\":\"%s\",\"ph\":\"B\",\"ts\":%lld,\"pid\":%d,\"tid\":%d},",
				result.Name, "cat", TimestampToMicroseconds(result.Timestamp), 0, 0);
			s_SessionDumpFile->Write((uint8*)eventBuffer, charsWritten);
			memset(eventBuffer, 0, 4096);
			// End event
			charsWritten = sprintf_s(eventBuffer, "{\"ph\":\"E\",\"ts\":%lld,\"pid\":%d,\"tid\":%d},",
				TimestampToMicroseconds(result.EndTimestamp), 0, 0);
			s_SessionDumpFile->Write((uint8*)eventBuffer, charsWritten);
		}

		s_TraceResults.clear();
		s_NamedTraceResults.clear();
	}

	void DebugTracing::ScopedTracer::CacheResult(int64 endTime, float duration)
	{
		ionassert(HasSessionStarted());
		float durationMs = duration * 0.001f;
		TraceResult& traceResult = s_TraceResults.emplace_back<TraceResult>({ m_Name, m_StartTime, endTime, duration });
		s_NamedTraceResults[m_Name] = &traceResult;

		if (s_TraceResults.size() >= ION_TRACE_DUMP_THRESHOLD)
		{
			DumpResults();
		}
	}

	const char* DebugTracing::s_CurrentSessionName = nullptr;
	bool DebugTracing::s_bSessionRecording = false;
	TArray<DebugTracing::TraceResult> DebugTracing::s_TraceResults;
	THashMap<String, DebugTracing::TraceResult*> DebugTracing::s_NamedTraceResults;
	FileOld* DebugTracing::s_SessionDumpFile = nullptr;
}

#endif
