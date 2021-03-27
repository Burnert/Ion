#include "IonPCH.h"

#include "OpenGL.h"

#ifdef ION_PLATFORM_WINDOWS
#include "RenderAPI/OpenGL/Windows/OpenGLWindows.h"
#endif

namespace Ion
{
    void OpenGL::InitOpenGL()
    {
#ifdef ION_PLATFORM_WINDOWS
        OpenGLWindows::InitOpenGL();
#else
        LOG_CRITICAL("OpenGL implementation is not defined on this platform!");
#endif
    }

    char OpenGL::s_DisplayName[120] = "OpenGL ";

    const char* OpenGL::GetDisplayName()
    {
        return s_DisplayName;
    }

    void OpenGL::SetDisplayVersion(const char* version)
    {
        strcpy_s((s_DisplayName + 7), 120 - 7, version);
    }
}
