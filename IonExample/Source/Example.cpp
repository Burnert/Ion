#include "IonPCH.h"

#include "Ion.h"

//DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop, "Main Loop", "Client");
//DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop_Section, "Main Loop Section", "Client");

//DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileI, "FileI");
//DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileIC, "FileIC");
//DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileCpp, "Fcpp");

class IonExample : public IonApplication
{
public:
	IonExample()
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}

	virtual void OnInit() override
	{
		//Ion::SerialisationTest();

		Ion::File* file = Ion::File::Create(TEXT("linetest.txt"));
		file->Open(Ion::IO::FM_Read);

		std::string line;
		while (!file->EndOfFile())
		{
			file->ReadLine(line);
			LOG_INFO("Read Line: {0}", line);
		}

// #include "FileTest"

	}

	virtual void OnUpdate(float DeltaTime) override
	{
		//{
		//	SCOPED_PERFORMANCE_COUNTER(Counter_MainLoop);

		//	using namespace std::chrono_literals;
		//	std::this_thread::sleep_for(0.2s);

		//	MANUAL_PERFORMANCE_COUNTER(Counter_MainLoop_Section);
		//	Counter_MainLoop_Section->Start();

		//	std::this_thread::sleep_for(0.2s);

		//	Counter_MainLoop_Section->Stop();
		//}

		//COUNTER_TIME_DATA(dataMainLoop, "Counter_MainLoop");
		//COUNTER_TIME_DATA(dataMainLoopSection, "Counter_MainLoop_Section");

		//LOG_WARN("{0} Data: {1}", dataMainLoop.Name, dataMainLoop.GetTimeMs());
		//LOG_WARN("{0} Data: {1}", dataMainLoopSection.Name, dataMainLoopSection.GetTimeMs());
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
