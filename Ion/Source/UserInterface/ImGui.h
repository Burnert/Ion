#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreMacros.h"

#define IMGUI_API ION_API
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui/imgui.h"

#if ION_ENGINE

// Force export ImGui Demo function symbols from the dll

#pragma comment(linker, "/export:?ShowDemoWindow@ImGui@@YAXPEA_N@Z")
#pragma comment(linker, "/export:?ShowUserGuide@ImGui@@YAXXZ")
#pragma comment(linker, "/export:?ShowAboutWindow@ImGui@@YAXPEA_N@Z")
#pragma comment(linker, "/export:?ShowStyleEditor@ImGui@@YAXPEAUImGuiStyle@@@Z")
#pragma comment(linker, "/export:?ShowFontSelector@ImGui@@YAXPEBD@Z")

#if ION_PLATFORM_WINDOWS
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#endif // ION_PLATFORM_WINDOWS
#if PLATFORM_SUPPORTS_OPENGL
#include "imgui/backends/imgui_impl_opengl3.h"
#endif // PLATFORM_SUPPORTS_OPENGL

#endif
