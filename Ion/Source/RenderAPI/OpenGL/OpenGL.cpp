#include "IonPCH.h"

#include "OpenGl.h"

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
}
