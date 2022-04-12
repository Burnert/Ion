#pragma once

#include "RHI/RHI.h"

#include "glad/glad.h"

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	class GenericWindow;

	class ION_API OpenGL : public RHI
	{
	public:
		/* Called by the Application class */
		virtual bool Init(GenericWindow* window) override;
		virtual bool InitWindow(GenericWindow& window) override;
		virtual void Shutdown() override;
		virtual void ShutdownWindow(GenericWindow& window) override;

		virtual void BeginFrame() override;
		virtual void EndFrame(GenericWindow& window) override;

		virtual void ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height) override;
		virtual void ResizeBuffers(GenericWindow& window, const TextureDimensions& size) override;

		virtual String GetCurrentDisplayName() override;

		static FORCEINLINE const char* GetVendor()           { return (const char*)glGetString(GL_VENDOR); }
		static FORCEINLINE const char* GetRendererName()     { return (const char*)glGetString(GL_RENDERER); }
		static FORCEINLINE const char* GetVersion()          { return (const char*)glGetString(GL_VERSION); }
		static FORCEINLINE const char* GetLanguageVersion()  { return (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION); }
		static FORCEINLINE const char* GetExtensions()       { return (const char*)glGetString(GL_EXTENSIONS); }

		static FORCEINLINE int32 GetMajorVersion() { return s_MajorVersion; }
		static FORCEINLINE int32 GetMinorVersion() { return s_MinorVersion; }

		const char* GetDisplayName() const;

		static void FilterDebugMessages();

		// Implemented per platform:
	public:
		static void SetSwapInterval(int32 interval);
		static int32 GetSwapInterval();

		// End Implemented per platform

	protected:
		static void SetDisplayVersion(const char* version);

	private:
		virtual void InitImGuiBackend() override;
		virtual void ImGuiNewFrame() override;
		virtual void ImGuiRender(ImDrawData* drawData) override;
		virtual void ImGuiShutdown() override;

		static void ImGuiImplRendererCreateWindowPlatform(ImGuiViewport* viewport);
		static void ImGuiImplRendererRenderWindow(ImGuiViewport* viewport, void*);
		static void ImGuiImplRendererSwapBuffers(ImGuiViewport* viewport, void*);
		static void ImGuiImplRendererDestroyWindow(ImGuiViewport* viewport);

	protected:
		static bool s_GLInitialized;
		static int32 s_MajorVersion;
		static int32 s_MinorVersion;

	private:
		static char s_DisplayName[120];

		friend class RHI;
	};
}
