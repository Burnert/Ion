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
		std::string line;

		f.Open(Ion::IO::FM_Read | Ion::IO::FM_Write);

		char bigBuffer[80];
		memset(bigBuffer, 'a', 80);
		f.WriteLine(bigBuffer, 80);
		f.WriteLine("abcdefgh2___");
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
		/*
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
		ASSERT(strcmp(bigBuffer4, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") == 0)

		f.SetOffset(0);
		std::string readStr;
		f.ReadLine(readStr);
		ASSERT(readStr.length() == 601)
		LOG_INFO("Line read: {0}", readStr);
		*/

		// My version to std::fstream comparison

		char cl1[16384];
		{
			SCOPED_PERFORMANCE_COUNTER(Counter_FileIC);

			for (int i = 0; i < 100000; ++i)
			{
				f.SetOffset(0);
				f.ReadLine(cl1, 16384);
			}
		}
		//LOG_INFO("Internal read CStr: {0}", cl1);

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
		//LOG_INFO("Internal read: {0}", l1);

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
		//LOG_INFO("C++ read: {0}", l2);

		COUNTER_TIME_DATA(fileTime2, "Counter_FileCpp");


		LOG_INFO("Internal CStr ReadLine: {0}", fileTimeC.GetTimeNs());
		LOG_INFO("Internal File ReadLine: {0}", fileTime.GetTimeNs());
		LOG_INFO("C++ File ReadLine:      {0}", fileTime2.GetTimeNs());
		LOG_INFO("Internal CStr:   {0}x faster than C++ fstream.", (float)fileTime2.GetTimeNs() / (float)fileTimeC.GetTimeNs());
		LOG_INFO("Internal String: {0}x faster than C++ fstream.", (float)fileTime2.GetTimeNs() / (float)fileTime.GetTimeNs());

		fs.close();

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
