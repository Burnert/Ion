#include "IonPCH.h"

#include "Core/StringConverter.h"

#pragma warning(disable:6386)

namespace Ion
{
	int StringConverter::WCharToChar(const wchar* wcstr, char* outCstr, int cstrLength)
	{
		ullong length = wcslen(wcstr);
		ionassert(length <= cstrLength, "Output buffer is too small to fit the converted string!");
		ullong strSize = (length + 1) * sizeof(char);
		return WideCharToMultiByte(CP_UTF8, 0, wcstr, -1, outCstr, (int)strSize, nullptr, nullptr);
	}

	int StringConverter::CharToWChar(const char* cstr, wchar* outWcstr, int wcstrLength)
	{
		ullong length = strlen(cstr);
		ionassert(length <= wcstrLength, "Output buffer is too small to fit the converted string!");
		ullong wcsSize = (length + 1) * sizeof(wchar);
		return MultiByteToWideChar(CP_UTF8, 0, cstr, -1, outWcstr, (int)wcsSize);
	}
}
