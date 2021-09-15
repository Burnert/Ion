#pragma once

#include "glad/glad.h"

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	class ION_API OpenGL
	{
		friend class RenderAPI;
	public:
		/* Called by the Application class */
		static void Init();

		static FORCEINLINE const char* GetVendor()           { return (const char*)glGetString(GL_VENDOR); }
		static FORCEINLINE const char* GetRendererName()     { return (const char*)glGetString(GL_RENDERER); }
		static FORCEINLINE const char* GetVersion()          { return (const char*)glGetString(GL_VERSION); }
		static FORCEINLINE const char* GetLanguageVersion()  { return (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION); }
		static FORCEINLINE const char* GetExtensions()       { return (const char*)glGetString(GL_EXTENSIONS); }

		static FORCEINLINE int GetMajorVersion() { return s_MajorVersion; }
		static FORCEINLINE int GetMinorVersion() { return s_MinorVersion; }

		static const char* GetDisplayName();

		// Implemented per platform:
	public:
		static void SetSwapInterval(int interval);
		static int GetSwapInterval();

		// End Implemented per platform

	protected:
		static void SetDisplayVersion(const char* version);

	private:
		static void InitImGuiBackend();
		static void ImGuiNewFrame();
		static void ImGuiRender(ImDrawData* drawData);
		static void ImGuiShutdown();

		static void ImGuiImplRendererCreateWindowPlatform(ImGuiViewport* viewport);
		static void ImGuiImplRendererRenderWindow(ImGuiViewport* viewport, void*);
		static void ImGuiImplRendererSwapBuffers(ImGuiViewport* viewport, void*);
		static void ImGuiImplRendererDestroyWindow(ImGuiViewport* viewport);

	protected:
		static bool s_GLInitialized;
		static int s_MajorVersion;
		static int s_MinorVersion;

	private:
		static char s_DisplayName[120];
	};
}
