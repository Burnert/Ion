#pragma once

#if ION_PLATFORM_WINDOWS
	#if ION_EDITOR_SHARED_LIB
		#if ION_EDITOR
			#define EDITOR_API __declspec(dllexport)
		#else
			#define EDITOR_API __declspec(dllimport)
		#endif
	#else
		#define EDITOR_API
	#endif
#else
	#error "Currently only Windows is supported!";
#endif
