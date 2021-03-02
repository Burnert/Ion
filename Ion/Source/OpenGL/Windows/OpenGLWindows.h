#pragma once

#include "OpenGL/OpenGL.h"

#include "glad/glad.h"

namespace Ion
{
	class OpenGLWindows : public OpenGL
	{
		friend class WindowsApplication;
		friend class WindowsWindow;

	public:
		static FORCEINLINE const char* GetVendor()           { return (const char*) glGetString(GL_VENDOR); }
		static FORCEINLINE const char* GetRendererName()     { return (const char*) glGetString(GL_RENDERER); }
		static FORCEINLINE const char* GetVersion()          { return (const char*) glGetString(GL_VERSION); }
		static FORCEINLINE const char* GetLanguageVersion()  { return (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION); }
		static FORCEINLINE const char* GetExtensions()       { return (const char*) glGetString(GL_EXTENSIONS); }

	protected:
		/* Called by the Application class */
		static void InitLoader();

		static void InitGLLoader();
		static void InitWGLLoader(HDC hdc);
		static void InitWGLExtensions();

		static void InitLibraries();
		static void FreeLibraries();

		static HGLRC CreateGLContext(HDC hdc);

		static void MakeContextCurrent(HDC hdc, HGLRC hglrc);

		static void* GetProcAddress(const char* name);

	private:
		static bool s_GLInitialized;
		static HMODULE s_OpenGLModule;
	};

	/* Dummy window used for creation of an OpenGL context and extensions. */
	class DummyWindow
	{
	public:
		~DummyWindow();

		HGLRC CreateFakeGLContext();

		void Destroy();

		FORCEINLINE HDC GetDeviceContext() const { return m_DeviceContext; }

	private:
		HWND m_WindowHandle;
		HDC m_DeviceContext;
		HGLRC m_RenderingContext;
	};
}
