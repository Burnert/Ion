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
		int status = gladLoadGLLoader((GLADloadproc)GetProcAddress);
		ASSERT(status);
	}

	void OpenGLWindows::InitWGLLoader(HDC hdc)
	{
		ASSERT(hdc);

		int status = gladLoadWGLLoader((GLADloadproc)GetProcAddress, hdc);
		ASSERT(status);
	}

	void OpenGLWindows::InitWGLExtensions()
	{
		InitLibraries();

		DummyWindow dummyWindow;

		HGLRC fakeContext = dummyWindow.CreateFakeGLContext();
		HDC hdc = dummyWindow.GetDeviceContext();

		InitGLLoader();
		InitWGLLoader(hdc);

		const char* vendor     = GetVendor();
		const char* renderer   = GetRendererName();
		const char* version    = GetVersion();
		const char* language   = GetLanguageVersion();
		LOG_INFO("       OpenGL Info:");
		LOG_INFO("Vendor:            {0}", vendor);
		LOG_INFO("Renderer:          {0}", renderer);
		LOG_INFO("OpenGL Version:    {0}", version);
		LOG_INFO("Language version:  {0}", language);

		FreeLibraries();
	}

	void OpenGLWindows::InitLoader()
	{
		InitWGLExtensions();
	}

	void OpenGLWindows::InitLibraries()
	{
		s_OpenGLModule = LoadLibrary(TEXT("opengl32.dll"));
		ASSERT(s_OpenGLModule);
	}

	void OpenGLWindows::FreeLibraries()
	{
		int status = FreeLibrary(s_OpenGLModule);
		ASSERT(status);
	}

	HGLRC OpenGLWindows::CreateGLContext(HDC hdc)
	{
		PIXELFORMATDESCRIPTOR pfd { };
		const int attributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0, // End
		};
		int pixelFormat;
		UINT numFormats;

		wglChoosePixelFormatARB(hdc, attributes, NULL, 1, &pixelFormat, &numFormats);
		ASSERT(pixelFormat != 0);

		int status = SetPixelFormat(hdc, pixelFormat, &pfd);
		ASSERT(status != 0);

		HGLRC renderingContext = wglCreateContext(hdc);
		ASSERT(renderingContext != NULL);

		return renderingContext;
	}

	void OpenGLWindows::MakeContextCurrent(HDC hdc, HGLRC hglrc)
	{
		wglMakeCurrent(hdc, hglrc);
	}

	void* OpenGLWindows::GetProcAddress(const char* name)
	{
		void* address = wglGetProcAddress(name);
		if (address)
			return address;

		return ::GetProcAddress(s_OpenGLModule, name);
	}

	HMODULE OpenGLWindows::s_OpenGLModule;
	bool OpenGLWindows::s_GLInitialized;


	// -----------------------------
	//         Dummy Window
	// -----------------------------

#ifdef UNICODE
	static inline const wchar* AppClassName = L"WinWGL";
#endif

	DummyWindow::~DummyWindow()
	{
		Destroy();
	}

	HGLRC DummyWindow::CreateFakeGLContext()
	{
		HINSTANCE hInstance = WindowsApplication::GetHInstance();

		WNDCLASS wc = { };
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = DefWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = AppClassName;

		if (!RegisterClass(&wc))
		{
			MessageBox(NULL, L"Cannot initialize OpenGL!\nWindows WNDCLASS Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		UINT32 WindowStyle = 0;

		m_WindowHandle = CreateWindowEx(
			0, // WindowStyleEx = None
			AppClassName,
			TEXT("WinWGLWindow"),
			0, // WindowStyle = None

			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0, 0, // Dimensions don't matter in this case.

			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (m_WindowHandle == NULL)
		{
			MessageBox(NULL, L"Cannot initialize OpenGL!\nContext creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		m_DeviceContext = GetDC(m_WindowHandle);

		PIXELFORMATDESCRIPTOR pfd {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int pixelFormat = ChoosePixelFormat(m_DeviceContext, &pfd);
		int result = SetPixelFormat(m_DeviceContext, 1, &pfd);
		ASSERT(result);

		m_RenderingContext = wglCreateContext(m_DeviceContext);
		ASSERT(m_RenderingContext);

		result = wglMakeCurrent(m_DeviceContext, m_RenderingContext);
		ASSERT(result);

		return m_RenderingContext;
	}

	void DummyWindow::Destroy()
	{
		wglDeleteContext(m_RenderingContext);
		DestroyWindow(m_WindowHandle);
	}
}
