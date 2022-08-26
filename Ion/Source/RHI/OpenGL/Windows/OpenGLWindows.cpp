#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_OPENGL

#include "OpenGLWindows.h"
#include "glad/glad_wgl.h"

#include "UserInterface/ImGui.h"

#pragma comment(lib, "opengl32.lib")

namespace Ion
{
	// -----------------------------
	//         OpenGLWindows
	// -----------------------------

	void OpenGLWindows::InitGLLoader()
	{
		TRACE_FUNCTION();

		ionverify(gladLoadGLLoader((GLADloadproc)GetProcAddress), "Could not load OpenGL!");
	}

	void OpenGLWindows::InitWGLLoader(HDC hdc)
	{
		TRACE_FUNCTION();

		ionassert(hdc);
		ionverify(gladLoadWGLLoader((GLADloadproc)GetProcAddress, hdc), "Could not load WGL extensions!");
	}

	void OpenGLWindows::InitOpenGL()
	{
		ionassert(!s_GLInitialized);

		InitLibraries();

		DummyWindow dummyWindow;
		HGLRC fakeContext = dummyWindow.CreateFakeGLContext();
		HDC hdc = dummyWindow.m_DeviceContext;

		InitGLLoader();
		InitWGLLoader(hdc);

		glDebugMessageCallback(OpenGLWindows::DebugCallback, nullptr);

		OpenGLLogger.Info("  OpenGL Info:");
		OpenGLLogger.Info("Vendor:            {0}", GetVendor());
		OpenGLLogger.Info("Renderer:          {0}", GetRendererName());
		OpenGLLogger.Info("OpenGL Version:    {0}", GetVersion());
		OpenGLLogger.Info("Language version:  {0}", GetLanguageVersion());

		glGetIntegerv(GL_MAJOR_VERSION, &s_MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &s_MinorVersion);

		OpenGL::SetDisplayVersion(GetVersion());

		FreeLibraries();

		wglMakeCurrent(0, 0);

		s_GLInitialized = true;
	}

	void OpenGLWindows::Init(RHIWindowData& window)
	{
		TRACE_FUNCTION();

		InitOpenGL();

		//WindowsWindow* windowsWindow = (WindowsWindow*)window;
		//windowsWindow->m_RenderingContext = CreateGLContext(windowsWindow->m_DeviceContext);

		//MakeContextCurrent(windowsWindow->m_DeviceContext, windowsWindow->m_RenderingContext);
	}

	void OpenGLWindows::InitLibraries()
	{
		TRACE_FUNCTION();

		s_OpenGLModule = LoadLibrary(TEXT("opengl32.dll"));
		ionverify(s_OpenGLModule, "Could not load opengl32.dll!");
	}

	void OpenGLWindows::FreeLibraries()
	{
		ionverify(FreeLibrary(s_OpenGLModule));
	}

	HGLRC OpenGLWindows::CreateGLContext(HDC hdc, HGLRC shareContext)
	{
		#pragma warning(disable:6011)
		#pragma warning(disable:6387)

		TRACE_FUNCTION();

		ionassert(s_GLInitialized);

		ionassert(hdc);

		ionverify(wglChoosePixelFormatARB);
		ionverify(wglCreateContextAttribsARB);
		ionverify(wglSwapIntervalEXT, "WGL_GLX_swap_control not found!");

		PIXELFORMATDESCRIPTOR pfd { };
		const int32 attributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0 // End
		};
		int32 pixelFormat = 0;
		UINT numFormats;

		TRACE_BEGIN(0, "OpenGLWindows - wglChoosePixelFormatARB");
		wglChoosePixelFormatARB(hdc, attributes, NULL, 1, &pixelFormat, &numFormats);
		ionverify(pixelFormat != 0, "No compatible pixel format exists!");
		TRACE_END(0);

		TRACE_BEGIN(1, "OpenGLWindows - Win32::SetPixelFormat");
		ionverify(SetPixelFormat(hdc, pixelFormat, &pfd), "Cannot set the pixel format!");
		TRACE_END(1);

		const int32 wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, s_MajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, s_MinorVersion,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 
#ifdef ION_DEBUG
			| WGL_CONTEXT_DEBUG_BIT_ARB
#endif
			,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0 // End
		};

		TRACE_BEGIN(2, "OpenGLWindows - wglCreateContextAttribsARB");
		HGLRC renderingContext = wglCreateContextAttribsARB(hdc, shareContext, wglAttributes);
		ionverify(renderingContext != NULL, "Cannot create an OpenGL rendering context!");
		TRACE_END(2);

		// Share the first context created
		// This will be needed when creating additional context, like ImGui windows
		if (!s_hShareContext)
		{
			s_hShareContext = renderingContext;
		}

		TRACE_BEGIN(3, "OpenGLWindows - wglMakeCurrent");
		ionverify(wglMakeCurrent(hdc, renderingContext));
		TRACE_END(3);

		glDebugMessageCallback(OpenGLWindows::DebugCallback, nullptr);

		FilterDebugMessages();

		return renderingContext;
	}

	void OpenGLWindows::MakeContextCurrent(HDC hdc, HGLRC hglrc)
	{
		TRACE_FUNCTION();

		ionverify(wglMakeCurrent(hdc, hglrc));
	}

	void* OpenGLWindows::GetProcAddress(const char* name)
	{
		void* address = wglGetProcAddress(name);
		if (address)
			return address;

		return ::GetProcAddress(s_OpenGLModule, name);
	}

	void OpenGLWindows::DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			OpenGLLogger.Critical("OpenGL Warning (High Severity): \n{0}", message);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			OpenGLLogger.Error("OpenGL Warning (Medium Severity): \n{0}", message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			OpenGLLogger.Warn("OpenGL Warning (Low Severity): \n{0}", message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			OpenGLLogger.Trace("OpenGL Notification: \n{0}", message);
			break;
		}
	}

	HMODULE OpenGLWindows::s_OpenGLModule = nullptr;
	HGLRC OpenGLWindows::s_hShareContext = nullptr;

	// -----------------------------
	//         Dummy Window
	// -----------------------------

	DummyWindow::~DummyWindow()
	{
		Destroy();
	}

	HGLRC DummyWindow::CreateFakeGLContext()
	{
		#pragma warning(disable:6387)

		TRACE_FUNCTION();

		const TCHAR* windowClassName = TEXT("DummyGLWindow");

		WNDCLASS wc = { };
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = DefWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = NULL;
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = windowClassName;

		TRACE_BEGIN(0, "DummyWindow - Win32::RegisterClass");
		if (!RegisterClass(&wc))
		{
			MessageBox(NULL, L"Cannot initialize OpenGL!\nWindows WNDCLASS Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
		TRACE_END(0);

		TRACE_BEGIN(1, "DummyWindow - Win32::CreateWindowEx");
		m_WindowHandle = CreateWindowEx(
			NULL, // WindowStyleEx = None
			windowClassName,
			NULL,
			NULL, // WindowStyle = None
			0, 0, // Window position
			1, 1, // Dimensions don't matter in this case.
			NULL, NULL, NULL, NULL);
		TRACE_END(1);

		if (m_WindowHandle == NULL)
		{
			MessageBox(NULL, L"Cannot initialize OpenGL!\nContext creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		m_DeviceContext = GetDC(m_WindowHandle);
		ionverify(m_DeviceContext);

		PIXELFORMATDESCRIPTOR pfd { };
		pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion   = 1;
		pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		TRACE_BEGIN(2, "DummyWindow - Win32::ChoosePixelFormat");
		int32 pixelFormat = ChoosePixelFormat(m_DeviceContext, &pfd);
		ionverify(pixelFormat);
		TRACE_END(2);

		TRACE_BEGIN(3, "DummyWindow - Win32::SetPixelFormat");
		ionverify(SetPixelFormat(m_DeviceContext, pixelFormat, &pfd));
		TRACE_END(3);

		TRACE_BEGIN(4, "DummyWindow - Win32::wglCreateContext");
		m_RenderingContext = wglCreateContext(m_DeviceContext);
		ionverify(m_RenderingContext);
		TRACE_END(4);

		TRACE_BEGIN(5, "DummyWindow - Win32::wglMakeCurrent");
		ionverify(wglMakeCurrent(m_DeviceContext, m_RenderingContext));
		TRACE_END(5);

		return m_RenderingContext;
	}

	void DummyWindow::Destroy()
	{
		TRACE_FUNCTION();

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_RenderingContext);
		ReleaseDC(m_WindowHandle, m_DeviceContext);
		DestroyWindow(m_WindowHandle);
	}

	// -----------------------------------
	// OpenGL implementation
	// -----------------------------------

	void OpenGL::SetSwapInterval(int32 interval)
	{
		int32 bResult = wglSwapIntervalEXT(interval);
		ionassert(bResult);
	}

	int32 OpenGL::GetSwapInterval()
	{
		return wglGetSwapIntervalEXT();
	}

	// ----------------------------------------------
	//    OpenGL ImGui Implementation
	// ----------------------------------------------

	void OpenGL::ImGuiImplRendererCreateWindowPlatform(ImGuiViewport* viewport)
	{
		TRACE_FUNCTION();

		ImGuiViewportDataOpenGLWin32* data = new ImGuiViewportDataOpenGLWin32;
		viewport->RendererUserData = data;

		HWND hWnd = (HWND)viewport->PlatformHandleRaw;

		data->DeviceContext = GetDC(hWnd);
		ionverify(data->DeviceContext);

		PIXELFORMATDESCRIPTOR pfd { };
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion     = 1;
		pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType   = PFD_TYPE_RGBA;
		pfd.cColorBits   = 32;
		pfd.cStencilBits = 8;
		pfd.cAlphaBits   = 8;
		pfd.cDepthBits   = 24;
		pfd.iLayerType   = PFD_MAIN_PLANE;

		int32 pixelFormat = ChoosePixelFormat(data->DeviceContext, &pfd);
		ionverify(pixelFormat);

		ionverify(SetPixelFormat(data->DeviceContext, pixelFormat, &pfd));

		const int32 wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, s_MajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, s_MinorVersion,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#ifdef ION_DEBUG
			| WGL_CONTEXT_DEBUG_BIT_ARB
#endif
			,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0 // End
		};

		data->RenderingContext = wglCreateContextAttribsARB(data->DeviceContext, OpenGLWindows::GetShareContext(), wglAttributes);
		ionverify(data->RenderingContext);

		ionverify(wglMakeCurrent(data->DeviceContext, data->RenderingContext));

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(OpenGLWindows::DebugCallback, nullptr);

		FilterDebugMessages();

		ionverify(wglSwapIntervalEXT(0));
	}

	void OpenGL::ImGuiImplRendererRenderWindow(ImGuiViewport* viewport, void*)
	{
		TRACE_FUNCTION();

		ImGuiViewportDataOpenGLWin32* data = (ImGuiViewportDataOpenGLWin32*)viewport->RendererUserData;

		ionverify(wglMakeCurrent(data->DeviceContext, data->RenderingContext));
	}

	void OpenGL::ImGuiImplRendererSwapBuffers(ImGuiViewport* viewport, void*)
	{
		TRACE_FUNCTION();

		ImGuiViewportDataOpenGLWin32* data = (ImGuiViewportDataOpenGLWin32*)viewport->RendererUserData;

		ionverify(SwapBuffers(data->DeviceContext));
	}

	void OpenGL::ImGuiImplRendererDestroyWindow(ImGuiViewport* viewport)
	{
		TRACE_FUNCTION();

		if (ImGuiViewportDataOpenGLWin32* data = (ImGuiViewportDataOpenGLWin32*)viewport->RendererUserData)
		{
			ionverify(wglDeleteContext(data->RenderingContext));

			ReleaseDC((HWND)viewport->PlatformHandleRaw, data->DeviceContext);

			delete data;
			viewport->RendererUserData = nullptr;
		}
	}
}

#endif // RHI_BUILD_OPENGL
