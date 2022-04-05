#pragma once

#if ION_PLATFORM_WINDOWS
	#if ION_SHARED_LIB
		#if ION_ENGINE
			#define ION_API __declspec(dllexport)
		#else
			#define ION_API __declspec(dllimport)
		#endif
	#else
		#define ION_API
	#endif
#else
	#error "Currently only Windows is supported!";
#endif

#if defined ION_DEBUG || ION_RELEASE
	#define ION_LOG_ENABLED 1
#endif
