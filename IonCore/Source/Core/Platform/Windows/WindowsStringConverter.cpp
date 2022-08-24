#include "Core/CorePCH.h"

#include "WindowsCore.h"
#include "Core/String/StringConverter.h"
#include "Core/Error/Error.h"

#pragma warning(disable:6386)

namespace Ion
{
	int32 StringConverter::W2MB(const wchar* wcStr, int32 wcStrLen, char* outBuffer, int32 bufferCount)
	{
		ionassert(wcStr);

		// Don't convert an empty string.
		if (wcStrLen == 0 || wcStrLen == -1 && wcslen(wcStr) == 0)
			return 0;

		ionassert((wcStrLen == -1) || wcStrLen == wcslen(wcStr));

		int32 minLength = WideCharToMultiByte(CP_UTF8, 0, wcStr, wcStrLen, nullptr, 0, nullptr, nullptr);

		if (!outBuffer)
			return minLength;

		ionverify(bufferCount >= minLength, "Output buffer is too small to fit the converted string.");

		int32 bytesWritten = WideCharToMultiByte(CP_UTF8, 0, wcStr, wcStrLen, outBuffer, bufferCount, nullptr, nullptr);
		ionverify(bytesWritten, "WideCharToMultiByte could not convert the string.\n{}", Windows::GetLastErrorMessage());
		return bytesWritten;
	}

	int32 StringConverter::MB2W(const char* mbStr, int32 mbStrLen, wchar* outBuffer, int32 bufferCount)
	{
		ionassert(mbStr);

		// Don't convert an empty string.
		if (mbStrLen == 0 || mbStrLen == -1 && strlen(mbStr) == 0)
			return 0;

		ionassert((mbStrLen == -1) || mbStrLen == strlen(mbStr));

		int32 minLength = MultiByteToWideChar(CP_UTF8, 0, mbStr, mbStrLen, nullptr, 0);

		if (!outBuffer)
			return minLength;

		ionverify(bufferCount >= minLength, "Output buffer is too small to fit the converted string.");

		int32 charsWritten = MultiByteToWideChar(CP_UTF8, 0, mbStr, mbStrLen, outBuffer, bufferCount);
		ionverify(charsWritten, "MultiByteToWideChar could not convert the string.\n{}", Windows::GetLastErrorMessage());
		return charsWritten;
	}
}
