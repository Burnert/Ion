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

		f.Open(Ion::IO::FM_Read | Ion::IO::FM_Write | Ion::IO::FM_Append);

		f.WriteLine("abcdefgh1___");

		byte fbuffer[4] = { 1, 6, 25, 66 };
		f.Write(fbuffer, 4);
		f.Write(fbuffer, 4);
		f.Write(fbuffer, 4);
		f.Write(fbuffer, 4);

		f.SetOffset(0);

		byte frbuffer[29];
		f.Read(frbuffer, 29);
		char sfrbuffer[30];
		memcpy_s(sfrbuffer, 29, frbuffer, 29);
		sfrbuffer[29] = '\0';

		LOG_INFO("Bytes read: {0}", sfrbuffer);
		f.Delete();
		f.SetOffset(-1);

		f.Close();

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
