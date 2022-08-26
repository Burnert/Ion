#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_OPENGL

#include "OpenGL.h"

#ifdef ION_PLATFORM_WINDOWS
#include "RHI/OpenGL/Windows/OpenGLWindows.h"
#endif

#include "UserInterface/ImGui.h"

namespace Ion
{
	Result<void, RHIError> OpenGL::Init(RHIWindowData& mainWindow)
	{
#ifdef ION_PLATFORM_WINDOWS
		OpenGLWindows::Init(mainWindow);
#else
		OpenGLLogger.Critical("OpenGL implementation is not defined on this platform!");
#endif
		return Ok();
	}

	Result<void, RHIError> OpenGL::InitWindow(RHIWindowData& window)
	{
		ionthrow(RHIError);
	}

	void OpenGL::Shutdown()
	{
	}

	void OpenGL::ShutdownWindow(RHIWindowData& window)
	{
	}

	Result<void, RHIError> OpenGL::BeginFrame()
	{
		return Ok();
	}

	Result<void, RHIError> OpenGL::EndFrame(RHIWindowData& window)
	{
		//window.SwapBuffers();
		return Ok();
	}

	Result<void, RHIError> OpenGL::ChangeDisplayMode(RHIWindowData& window, EWindowDisplayMode mode, uint32 width, uint32 height)
	{
		return Ok();
	}

	Result<void, RHIError> OpenGL::ResizeBuffers(RHIWindowData& window, const TextureDimensions& size)
	{
		return Ok();
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

#endif // RHI_BUILD_OPENGL
