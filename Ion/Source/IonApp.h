#pragma once

#include "Core/Core.h"

#ifdef ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
using IonApplication = Ion::WindowsApplication;
#endif

// Entry point ----------------
#include "Application/EntryPoint.h"

// Define before including the file
#ifndef DISABLE_USING_NAMESPACE_ION
using namespace Ion;
#endif
