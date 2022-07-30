#pragma once

namespace Ion::Platform
{
	enum EMessageBoxType : uint32
	{ // @TODO: Make this a bit different than in Windows
		MBT_Ok                = 0,
		MBT_OkCancel          = 1,
		MBT_AbortRetryIgnore  = 2,
		MBT_YesNoCancel       = 3,
		MBT_YesNo             = 4,
		MBT_RetryCancel       = 5,
		MBT_CancelTryContinue = 6,
	};

	enum EMessageBoxIcon : uint32
	{ // @TODO: Make this a bit different than in Windows
		MBI_Error    = 0x10,
		MBI_Question = 0x20,
		MBI_Warning  = 0x30,
		MBI_Asterisk = 0x40,
		MBI_User     = 0x80,
	};

	int32 MessageBox(const WString& text, const WString& caption,
		EMessageBoxType type, EMessageBoxIcon icon);

	int32 GetCurrentProcessId();
	int32 GetCurrentThreadId();
	void SetCurrentThreadDescription(const WString& desc);
	WString GetCurrentThreadDescription();
	bool IsMainThread();

	WString GetSystemDefaultFontPath();

	namespace _Detail
	{
		void SetMainThreadId();
	}
}
