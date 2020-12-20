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