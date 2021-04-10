#pragma once

#include "imgui/imgui.h"

#ifdef ION_PLATFORM_WINDOWS
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#endif
#include "imgui/backends/imgui_impl_opengl3.h"
