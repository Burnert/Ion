#pragma once

#include "RenderAPI/OpenGL/OpenGL.h"

#include "glad/glad.h"

#pragma comment(lib, "opengl32.lib")

namespace Ion
{
	struct ImGuiViewportDataOpenGLWin32
	{
		HDC DeviceContext;
		HGLRC RenderingContext;
	};

	class ION_API OpenGLWindows : public OpenGL
	{
		friend class WindowsApplication;
		friend class WindowsWindow;

	public:
		/* Called by the Application class */
		static void Init(GenericWindow* window);

		static HGLRC CreateGLContext(HDC hdc, HGLRC shareContext = nullptr);
		static void MakeContextCurrent(HDC hdc, HGLRC hglrc);

		FORCEINLINE static HGLRC GetShareContext() { return s_hShareContext; }

		static void APIENTRY DebugCallback(
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam);

	protected:
		static void InitOpenGL();

		static void InitGLLoader();
		static void InitWGLLoader(HDC hdc);

		static void InitLibraries();
		static void FreeLibraries();

		static void* GetProcAddress(const char* name);

	private:
		static HMODULE s_OpenGLModule;
		static HGLRC s_hShareContext;
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
