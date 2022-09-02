#pragma once

/* Specifies the main class of the application (can be used only once) */
#define ION_DEFINE_MAIN_APPLICATION_CLASS(className) \
namespace Ion { \
	inline IApp* const g_pClientApplication = new className; \
}

namespace Ion {
	extern int32 MainShared(int32 argc, wchar* argv[]);
}

#if UNICODE
int wmain(int argc, wchar_t* argv[])
{
	return Ion::MainShared(argc, argv);
}
#else
int main(int argc, char* argv[])
{
	return Ion::MainShared(argc, argv);
}
#endif
