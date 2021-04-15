#pragma once

#ifdef ION_PLATFORM_WINDOWS
	#ifdef ION_ENGINE
		#define ION_API __declspec(dllexport)
	#else
		#define ION_API __declspec(dllimport)
	#endif
#else
	#error "Currently only Windows is supported!";
#endif

#if defined ION_DEBUG || ION_RELEASE
	#define ION_LOG_ENABLED 1
#endif
