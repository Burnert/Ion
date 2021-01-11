#include "IonPCH.h"

#include "Ion.h"

#include <fstream>

DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop, "Main Loop", "Client");
DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop_Section, "Main Loop Section", "Client");

DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileI, "FileI");
DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileIC, "FileIC");
DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileCpp, "Fcpp");

class IonExample : public IonApplication
{
public:
	IonExample()
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}

	virtual void OnInit() override
	{
		Ion::File f(TEXT("testfile.txt"));
#ifdef ION_DEBUG
		//f.EnableDebugLog();
#endif
		std::string line;
		f.Delete();
		f.Open(Ion::IO::FM_Read | Ion::IO::FM_Write);

		char bigBuffer[81];
		memset(bigBuffer, 'a', 80);
		bigBuffer[80] = '\0';
		f.WriteLine(bigBuffer, 81);
		ASSERT(f.GetSize() == 81)
		f.WriteLine("abcdefgh2___");
		ASSERT(f.GetSize() == 81 + 13)
		f.WriteLine("abcdefgh3___");

		f.SetOffset(0);

		byte frbuffer[55];
		ZeroMemory(frbuffer, 55);
		f.Read(frbuffer, 55);
		char sfrbuffer[56];
		memcpy_s(sfrbuffer, 55, frbuffer, 55);
		sfrbuffer[55] = '\0';

		LOG_INFO("Bytes read: {0}", sfrbuffer);
		f.Delete();
		f.SetOffset(0);
		
		// Some tests just to make sure
#if 0 
		char bigBuffer2[256];
		f.SetOffset(0);
		ASSERT(f.ReadLine(bigBuffer2, 256))
		ASSERT(strcmp(bigBuffer2, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0)
		f.SetOffset(0);
		char bigBuffer3[601];
		ASSERT(f.ReadLine(bigBuffer3, 601))
		ASSERT(strcmp(bigBuffer3, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0)
		f.SetOffset(0);
		char bigBuffer4[602];
		ASSERT(f.ReadLine(bigBuffer4, 602))
		ASSERT(strcmp(bigBuffer4, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0)
		ASSERT(f.ReadLine(bigBuffer4, 602))
		ASSERT(strcmp(bigBuffer4, "abcdefgh2___") == 0)
		ASSERT(f.ReadLine(bigBuffer4, 602))
		ASSERT(strcmp(bigBuffer4, "abcdefgh3___") == 0)

		f.SetOffset(0);
		std::string readStr;
		f.ReadLine(readStr);
		ASSERT(readStr.length() == 600)
		LOG_INFO("Line read: {0}", readStr);
#endif

		// My version to std::fstream comparison

		char cl1[512];
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileIC);

			for (int i = 0; i < 100000; ++i)
			{
				f.SetOffset(0);
				f.ReadLine(cl1, 512);
			}
		}
		LOG_INFO("Internal read CStr: {0}", cl1);

		COUNTER_TIME_DATA(fileTimeC, "Counter_FileIC");


		std::string l1;
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileI);

			for (int i = 0; i < 100000; ++i)
			{
				f.SetOffset(0);
				f.ReadLine(l1);
			}
		}
		LOG_INFO("Internal read: {0}", l1);

		COUNTER_TIME_DATA(fileTime, "Counter_FileI");


		f.Close();

		std::string l2;
		std::fstream fs("testfile.txt", std::ios::in | std::ios::out);
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileCpp);

			for (int i = 0; i < 100000; ++i)
			{
				fs.seekg(0);
				std::getline(fs, l2);
			}
		}
		LOG_INFO("C++ read: {0}", l2);

		COUNTER_TIME_DATA(fileTime2, "Counter_FileCpp");


		LOG_INFO("Internal CStr ReadLine: {0}", fileTimeC.GetTimeNs());
		LOG_INFO("Internal File ReadLine: {0}", fileTime.GetTimeNs());
		LOG_INFO("C++ File ReadLine:      {0}", fileTime2.GetTimeNs());
		LOG_INFO("Internal CStr:   {0}x faster than C++ fstream.", (float)fileTime2.GetTimeNs() / (float)fileTimeC.GetTimeNs());
		LOG_INFO("Internal String: {0}x faster than C++ fstream.", (float)fileTime2.GetTimeNs() / (float)fileTime.GetTimeNs());

		fs.close();

		// Write line comparison

		f.Open(Ion::IO::FM_Read | Ion::IO::FM_Write);

		char cl2[81];
		memset(cl2, 'g', 80);
		cl2[80] = '\0';
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileIC);

			for (int i = 0; i < 100000; ++i)
			{
				f.SetOffset(0);
				f.WriteLine(cl2, 81);
			}
		}
		LOG_INFO("Internal write CStr: {0}", cl2);

		COUNTER_TIME_DATA(fileTimeCw, "Counter_FileIC");


		std::string lw1(cl2);
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileI);

			for (int i = 0; i < 100000; ++i)
			{
				f.SetOffset(0);
				f.WriteLine(lw1);
			}
		}
		LOG_INFO("Internal write: {0}", lw1);

		COUNTER_TIME_DATA(fileTimew, "Counter_FileI");


		f.Close();

		std::string lw2(cl2);
		std::fstream fs2("testfile.txt", std::ios::in | std::ios::out);
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileCpp);

			for (int i = 0; i < 100000; ++i)
			{
				fs2.seekg(0);
				fs2 << lw2 << '\n';
			}
		}
		LOG_INFO("C++ write: {0}", lw2);

		COUNTER_TIME_DATA(fileTime2w, "Counter_FileCpp");


		LOG_INFO("Internal CStr WriteLine: {0}", fileTimeCw.GetTimeNs());
		LOG_INFO("Internal File WriteLine: {0}", fileTimew.GetTimeNs());
		LOG_INFO("C++ File WriteLine:      {0}", fileTime2w.GetTimeNs());
		LOG_INFO("Internal CStr:   {0}x faster than C++ fstream.", (float)fileTime2w.GetTimeNs() / (float)fileTimeCw.GetTimeNs());
		LOG_INFO("Internal String: {0}x faster than C++ fstream.", (float)fileTime2w.GetTimeNs() / (float)fileTimew.GetTimeNs());

		fs2.close();

		f.Delete();

		LOG_TRACE("File done -------------------");
	}

	virtual void OnUpdate(float DeltaTime) override
	{
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_MainLoop);

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(0.2s);

			MANUAL_PERFORMANCE_COUNTER(Counter_MainLoop_Section);
			Counter_MainLoop_Section->Start();

			std::this_thread::sleep_for(0.2s);

			Counter_MainLoop_Section->Stop();
		}

		COUNTER_TIME_DATA(dataMainLoop, "Counter_MainLoop");
		COUNTER_TIME_DATA(dataMainLoopSection, "Counter_MainLoop_Section");

		LOG_WARN("{0} Data: {1}", dataMainLoop.Name, dataMainLoop.GetTimeMs());
		LOG_WARN("{0} Data: {1}", dataMainLoopSection.Name, dataMainLoopSection.GetTimeMs());
	}

	virtual void OnRender() override
	{
	}

	virtual void OnShutdown() override
	{
	}

	virtual void OnClientEvent(Ion::Event& event) override
	{
	}
};

USE_APPLICATION_CLASS(IonExample)
