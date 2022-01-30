#pragma once

#ifdef ION_PLATFORM_WINDOWS
	#ifdef ION_EDITOR
		#define EDITOR_API __declspec(dllexport)
	#else
		#define EDITOR_API __declspec(dllimport)
	#endif
#else
	#error "Currently only Windows is supported!";
#endif
