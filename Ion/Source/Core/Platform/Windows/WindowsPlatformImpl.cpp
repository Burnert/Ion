#include "IonPCH.h"

#include "Core/Platform/Platform.h"
#include "WindowsHeaders.h"

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
		return ::MessageBox(nullptr, text.c_str(), caption.c_str(),
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
}
