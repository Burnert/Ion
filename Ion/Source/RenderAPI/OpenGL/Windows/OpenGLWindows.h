#pragma once

#include "RenderAPI/OpenGL/OpenGL.h"

#include "glad/glad.h"

namespace Ion
{
	class OpenGLWindows : public OpenGL
	{
		friend class WindowsApplication;
		friend class WindowsWindow;

	public:
		/* Called by the Application class */
		static void InitOpenGL();

		static HGLRC CreateGLContext(HDC hdc);
		static void MakeContextCurrent(HDC hdc, HGLRC hglrc);

	protected:
		static void InitGLLoader();
		static void InitWGLLoader(HDC hdc);

		static void InitLibraries();
		static void FreeLibraries();

		static void* GetProcAddress(const char* name);

		static void DebugCallback(
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam);

	private:
		static bool s_GLInitialized;
		static HMODULE s_OpenGLModule;

		static int s_MajorVersion;
		static int s_MinorVersion;
	};

	/* Dummy window used for creation of an OpenGL context and extensions. */
	class DummyWindow
	{
		friend class OpenGLWindows;
	public:
		~DummyWindow();

		HGLRC CreateFakeGLContext();

		void Destroy();

	private:
		HWND m_WindowHandle;
		HDC m_DeviceContext;
		HGLRC m_RenderingContext;
	};
}
