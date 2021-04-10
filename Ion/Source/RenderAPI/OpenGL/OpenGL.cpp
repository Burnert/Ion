#include "IonPCH.h"

#include "OpenGL.h"

#ifdef ION_PLATFORM_WINDOWS
#include "RenderAPI/OpenGL/Windows/OpenGLWindows.h"
#endif

#include "UserInterface/ImGui.h"

namespace Ion
{
	void OpenGL::InitOpenGL()
	{
#ifdef ION_PLATFORM_WINDOWS
		OpenGLWindows::InitOpenGL();
#else
		LOG_CRITICAL("OpenGL implementation is not defined on this platform!");
#endif
	}

	char OpenGL::s_DisplayName[120] = "OpenGL ";

	bool OpenGL::s_GLInitialized = false;
	int OpenGL::s_MajorVersion = 4;
	int OpenGL::s_MinorVersion = 3;

	const char* OpenGL::GetDisplayName()
	{
		return s_DisplayName;
	}

	void OpenGL::SetDisplayVersion(const char* version)
	{
		strcpy_s((s_DisplayName + 7), 120 - 7, version);
	}

	void OpenGL::InitImGuiBackend()
	{
		ImGui_ImplOpenGL3_Init();

		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();

		platformIO.Renderer_CreateWindow  = ImGuiImplRendererCreateWindowPlatform;
		platformIO.Renderer_DestroyWindow = ImGuiImplRendererDestroyWindow;
		platformIO.Renderer_SwapBuffers   = ImGuiImplRendererSwapBuffers;

		platformIO.Platform_RenderWindow  = ImGuiImplRendererRenderWindow;
	}

	void OpenGL::ImGuiNewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
	}

	void OpenGL::ImGuiRender(ImDrawData* drawData)
	{
		ImGui_ImplOpenGL3_RenderDrawData(drawData);
	}

	void OpenGL::ImGuiShutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}
}
