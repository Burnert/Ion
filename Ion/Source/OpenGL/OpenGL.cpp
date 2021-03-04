#include "IonPCH.h"

#include "OpenGl.h"
#include "OpenGL/Windows/OpenGLWindows.h"

namespace Ion
{
    void OpenGL::InitOpenGL()
    {
#ifdef ION_PLATFORM_WINDOWS
        OpenGLWindows::InitOpenGL();
#endif
    }
}
