#pragma once

#include "glad/glad.h"

namespace Ion
{
	class ION_API OpenGL
	{
	public:
		/* Called by the Application class */
		static void InitOpenGL();

		static FORCEINLINE const char* GetVendor()           { return (const char*)glGetString(GL_VENDOR); }
		static FORCEINLINE const char* GetRendererName()     { return (const char*)glGetString(GL_RENDERER); }
		static FORCEINLINE const char* GetVersion()          { return (const char*)glGetString(GL_VERSION); }
		static FORCEINLINE const char* GetLanguageVersion()  { return (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION); }
		static FORCEINLINE const char* GetExtensions()       { return (const char*)glGetString(GL_EXTENSIONS); }

		static const char* GetDisplayName();

		// Implemented per platform:
	public:
		static void SetSwapInterval(int interval);
		static int GetSwapInterval();

		// End Implemented per platform

	protected:
		static void SetDisplayVersion(const char* version);

	private:
		static char s_DisplayName[120];
	};
}
