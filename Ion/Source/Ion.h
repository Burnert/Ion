#pragma once

#include "Core/Core.h"

// High level related things here...

#ifdef ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
using IonApplication = Ion::WindowsApplication;
#endif

// Define before including the file
#ifndef DISABLE_USING_NAMESPACE_ION
using namespace Ion;
#endif
