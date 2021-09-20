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

		int nameLength = (int)strlen(name);
		ionassert(nameLength < 50);

		bool bResult;

		ionassert(!HasSessionStarted());
		s_CurrentSessionName = name;

		wchar* nameW = (wchar*)_alloca((nameLength + 1) * sizeof(wchar));
		StringConverter::CharToWChar(name, nameW, nameLength);

		// @TODO: Make a date reading utility and fill the Xs in the filename
		wchar filename[100];
		memset(filename, 0, sizeof(filename));
		swprintf_s(filename, L"Trace_XX-XX-XX_XX-XX-XXXX_%s.json", nameW);

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
			int charsWritten;
			char eventBuffer[1000];
			memset(eventBuffer, 0, 1000);
			// Begin event
			charsWritten = sprintf_s(eventBuffer, "{\"name\":\"%s\",\"cat\":\"%s\",\"ph\":\"B\",\"ts\":%lld,\"pid\":%d,\"tid\":%d},",
				result.Name, "cat", TimestampToMicroseconds(result.Timestamp), 0, 0);
			s_SessionDumpFile->Write((ubyte*)eventBuffer, charsWritten);
			memset(eventBuffer, 0, 1000);
			// End event
			charsWritten = sprintf_s(eventBuffer, "{\"ph\":\"E\",\"ts\":%lld,\"pid\":%d,\"tid\":%d},",
				TimestampToMicroseconds(result.EndTimestamp), 0, 0);
			s_SessionDumpFile->Write((ubyte*)eventBuffer, charsWritten);
		}

		s_TraceResults.clear();
		s_NamedTraceResults.clear();
	}

	void DebugTracing::ScopedTracer::CacheResult(llong endTime, float duration)
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
	std::vector<DebugTracing::TraceResult> DebugTracing::s_TraceResults;
	std::unordered_map<String, DebugTracing::TraceResult*> DebugTracing::s_NamedTraceResults;
	File* DebugTracing::s_SessionDumpFile = nullptr;
}

#endif