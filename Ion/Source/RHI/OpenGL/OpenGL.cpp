#include "IonPCH.h"

#include "OpenGL.h"

#ifdef ION_PLATFORM_WINDOWS
#include "RHI/OpenGL/Windows/OpenGLWindows.h"
#endif

#include "UserInterface/ImGui.h"

#include "Application/Window/GenericWindow.h"

namespace Ion
{
	bool OpenGL::Init(GenericWindow* window)
	{
#ifdef ION_PLATFORM_WINDOWS
		OpenGLWindows::Init(window);
#else
		OpenGLLogger.Critical("OpenGL implementation is not defined on this platform!");
#endif
		return true;
	}

	bool OpenGL::InitWindow(GenericWindow& window)
	{
		return false;
	}

	void OpenGL::Shutdown()
	{
	}

	void OpenGL::ShutdownWindow(GenericWindow& window)
	{
	}

	void OpenGL::BeginFrame()
	{
	}

	void OpenGL::EndFrame(GenericWindow& window)
	{
		window.SwapBuffers();
	}

	void OpenGL::ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height)
	{
	}

	void OpenGL::ResizeBuffers(GenericWindow& window, const TextureDimensions& size)
	{
	}

	String OpenGL::GetCurrentDisplayName()
	{
		return GetDisplayName();
	}

	char OpenGL::s_DisplayName[120] = "OpenGL ";

	bool OpenGL::s_GLInitialized = false;
	int32 OpenGL::s_MajorVersion = 4;
	int32 OpenGL::s_MinorVersion = 3;

	const char* OpenGL::GetDisplayName() const
	{
		return s_DisplayName;
	}

	void OpenGL::FilterDebugMessages()
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_FALSE);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_FALSE);
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
