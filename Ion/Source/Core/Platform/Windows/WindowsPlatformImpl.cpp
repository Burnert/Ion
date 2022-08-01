#include "IonPCH.h"

#include "Core/Platform/Platform.h"
#include "WindowsHeaders.h"

#undef MessageBox

namespace Ion::Platform
{
	static UINT MessageBoxTypeToWindowsType(EMessageBoxType type)
	{
		return type;
	}

	static UINT MessageBoxIconToWindowsType(EMessageBoxIcon icon)
	{
		return icon;
	}

	int32 MessageBox(const WString& text, const WString& caption,
		EMessageBoxType type, EMessageBoxIcon icon)
	{
		return ::MessageBoxW(nullptr, text.c_str(), caption.c_str(),
			MessageBoxTypeToWindowsType(type) | MessageBoxIconToWindowsType(icon));
	}

	int32 GetCurrentProcessId()
	{
		return ::GetCurrentProcessId();
	}

	int32 GetCurrentThreadId()
	{
		return ::GetCurrentThreadId();
	}

	void SetCurrentThreadDescription(const WString& desc)
	{
		::SetThreadDescription(::GetCurrentThread(), desc.c_str());
	}

	WString GetCurrentThreadDescription()
	{
		wchar* desc;
		::GetThreadDescription(::GetCurrentThread(), &desc);
		WString descStr = desc;
		::LocalFree(desc);
		return descStr;
	}

	static DWORD g_MainThreadId = 0;

	bool IsMainThread()
	{
		return g_MainThreadId == ::GetCurrentThreadId();
	}

	WString GetSystemDefaultFontPath()
	{
		constexpr wchar* Paths[] = {
			L"C:\\Windows\\Fonts\\arial.ttf",
			L"C:\\Windows\\Fonts\\segoeui.ttf",
		};

		for (int32 i = 0; i < sizeof(Paths) / sizeof(wchar*); ++i)
		{
			if (FilePath(Paths[i]).Exists())
				return Paths[i];
		}

		return L"";
	}

	namespace _Detail
	{
		void SetMainThreadId()
		{
			g_MainThreadId = ::GetCurrentThreadId();
		}
	}
}
