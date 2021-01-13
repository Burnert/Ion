#pragma once

#define WINDOWS_FORMAT_ERROR_MESSAGE(msgVarName, error) \
WCHAR _##msgVarName[512]; \
FormatMessage( \
	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
	NULL, lastError, 0, _##msgVarName, 512, NULL); \
std::wstring msgVarName = _##msgVarName
