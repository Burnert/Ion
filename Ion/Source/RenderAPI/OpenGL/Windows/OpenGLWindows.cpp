#include "IonPCH.h"

#include "OpenGLWindows.h"
#include "Application/Platform/Windows/WindowsApplication.h"

#include "glad/glad_wgl.h"

namespace Ion
{
	// -----------------------------
	//         OpenGLWindows
	// -----------------------------

	void OpenGLWindows::InitGLLoader()
	{
		VERIFY(gladLoadGLLoader((GLADloadproc)GetProcAddress));
	}

	void OpenGLWindows::InitWGLLoader(HDC hdc)
	{
		ASSERT(hdc);
		VERIFY(gladLoadWGLLoader((GLADloadproc)GetProcAddress, hdc));
	}

	void OpenGLWindows::InitOpenGL()
	{
		InitLibraries();

		DummyWindow dummyWindow;
		HGLRC fakeContext = dummyWindow.CreateFakeGLContext();
		HDC hdc = dummyWindow.m_DeviceContext;

		InitGLLoader();
		InitWGLLoader(hdc);

		glDebugMessageCallback(OpenGLWindows::DebugCallback, nullptr);

		LOG_INFO("  OpenGL Info:");
		LOG_INFO("Vendor:            {0}", GetVendor());
		LOG_INFO("Renderer:          {0}", GetRendererName());
		LOG_INFO("OpenGL Version:    {0}", GetVersion());
		LOG_INFO("Language version:  {0}", GetLanguageVersion());

		glGetIntegerv(GL_MAJOR_VERSION, &s_MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &s_MinorVersion);

		OpenGL::SetDisplayVersion(GetVersion());

		dummyWindow.Destroy();
		FreeLibraries();

		s_GLInitialized = true;
	}

	void OpenGLWindows::InitLibraries()
	{
		s_OpenGLModule = LoadLibrary(TEXT("opengl32.dll"));
		VERIFY_M(s_OpenGLModule, "Cannot load OpenGL!");
	}

	void OpenGLWindows::FreeLibraries()
	{
		VERIFY(FreeLibrary(s_OpenGLModule));
	}

	HGLRC OpenGLWindows::CreateGLContext(HDC hdc)
	{
		#pragma warning(disable:6011)

		ASSERT(hdc);

		VERIFY(wglChoosePixelFormatARB);
		VERIFY(wglCreateContextAttribsARB);

		VERIFY_M(wglSwapIntervalEXT, "WGL_GLX_swap_control not found!");

		PIXELFORMATDESCRIPTOR pfd { };
		const int attributes[] = {
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
		int pixelFormat = 0;
		UINT numFormats;

		wglChoosePixelFormatARB(hdc, attributes, NULL, 1, &pixelFormat, &numFormats);
		VERIFY(pixelFormat != 0);

		VERIFY(SetPixelFormat(hdc, pixelFormat, &pfd));

		const int wglAttributes[] = {
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

		HGLRC renderingContext = wglCreateContextAttribsARB(hdc, NULL, wglAttributes);
		VERIFY(renderingContext != NULL);

		VERIFY(wglMakeCurrent(hdc, renderingContext));

		glDebugMessageCallback(OpenGLWindows::DebugCallback, nullptr);

		return renderingContext;
	}

	void OpenGLWindows::MakeContextCurrent(HDC hdc, HGLRC hglrc)
	{
		VERIFY(wglMakeCurrent(hdc, hglrc));
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
			LOG_CRITICAL("OpenGL Critical Error: \n{0}", message);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			LOG_ERROR("OpenGL Error: \n{0}", message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			LOG_WARN("OpenGL Warning: \n{0}", message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			LOG_TRACE("OpenGL Notification: \n{0}", message);
			break;
		}
	}

	HMODULE OpenGLWindows::s_OpenGLModule = NULL;
	bool OpenGLWindows::s_GLInitialized = false;

	int OpenGLWindows::s_MajorVersion = 4;
	int OpenGLWindows::s_MinorVersion = 3;


	// -----------------------------
	//         Dummy Window
	// -----------------------------

	DummyWindow::~DummyWindow()
	{
		Destroy();
	}

	HGLRC DummyWindow::CreateFakeGLContext()
	{
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

		if (!RegisterClass(&wc))
		{
			MessageBox(NULL, L"Cannot initialize OpenGL!\nWindows WNDCLASS Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		m_WindowHandle = CreateWindowEx(
			NULL, // WindowStyleEx = None
			windowClassName,
			NULL,
			NULL, // WindowStyle = None
			0, 0, // Window position
			1, 1, // Dimensions don't matter in this case.
			NULL, NULL, NULL, NULL);

		if (m_WindowHandle == NULL)
		{
			MessageBox(NULL, L"Cannot initialize OpenGL!\nContext creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		m_DeviceContext = GetDC(m_WindowHandle);
		VERIFY(m_DeviceContext);

		PIXELFORMATDESCRIPTOR pfd { };
		pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion   = 1;
		pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat = ChoosePixelFormat(m_DeviceContext, &pfd);
		VERIFY(pixelFormat);

		VERIFY(SetPixelFormat(m_DeviceContext, pixelFormat, &pfd));

		m_RenderingContext = wglCreateContext(m_DeviceContext);
		VERIFY(m_RenderingContext);

		VERIFY(wglMakeCurrent(m_DeviceContext, m_RenderingContext));

		return m_RenderingContext;
	}

	void DummyWindow::Destroy()
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_RenderingContext);
		ReleaseDC(m_WindowHandle, m_DeviceContext);
		DestroyWindow(m_WindowHandle);
	}

	// -----------------------------------
	// OpenGL implementation
	// -----------------------------------

	void OpenGL::SetSwapInterval(int interval)
	{
		VERIFY(wglSwapIntervalEXT(interval));
	}

	int OpenGL::GetSwapInterval()
	{
		return wglGetSwapIntervalEXT();
	}
}
