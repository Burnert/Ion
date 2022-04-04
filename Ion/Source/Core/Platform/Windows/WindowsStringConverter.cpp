#include "IonPCH.h"

#include "Core/Platform/Windows/WindowsHeaders.h"
#include "Core/StringConverter.h"
#include "Core/CoreAsserts.h"

#pragma warning(disable:6386)

namespace Ion
{
	int32 StringConverter::WCharToChar(const wchar* wcstr, char* outCstr, int32 cstrLength)
	{
		uint64 length = wcslen(wcstr);
		ionassert(length <= cstrLength, "Output buffer is too small to fit the converted string!");
		uint64 strSize = (length + 1) * sizeof(char);
		return WideCharToMultiByte(CP_UTF8, 0, wcstr, -1, outCstr, (int32)strSize, nullptr, nullptr);
	}

	int32 StringConverter::CharToWChar(const char* cstr, wchar* outWcstr, int32 wcstrLength)
	{
		uint64 length = strlen(cstr);
		ionassert(length <= wcstrLength, "Output buffer is too small to fit the converted string!");
		uint64 wcsSize = (length + 1) * sizeof(wchar);
		return MultiByteToWideChar(CP_UTF8, 0, cstr, -1, outWcstr, (int32)wcsSize);
	}
}
