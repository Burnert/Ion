#include "IonPCH.h"

#include "Ion.h"

DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop, "Main Loop", "Client");
DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop_Section, "Main Loop Section", "Client");

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
		f.WriteLine("abcdefgh1___");
		f.ReadLine(line);
		f.AddOffset(-2000);
		f.Close();

		LOG_DEBUG("File {0}", f.Exists() ? "exists" : "does not exist");

		Ion::File f2;
		f2.Open(Ion::IO::FM_Read);
		LOG_DEBUG("File is {0}", f2.IsOpen() ? "open" : "closed");
		f2.Close();

		f2.Delete();
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

		LOG_WARN("{0} Data: {1}", dataMainLoop.Name, dataMainLoop.GetTimeNs());
		LOG_WARN("{0} Data: {1}", dataMainLoopSection.Name, dataMainLoopSection.GetTimeNs());
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
