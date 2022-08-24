#pragma once

#include "WindowsHeaders.h"
#include "Core/Base.h"
#include "Core/String/StringConverter.h"

#include <comdef.h>

namespace Ion::Windows
{
	inline String FormatErrorMessage(DWORD error)
	{
		TCHAR* pMessage = nullptr;

		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL, error, 0, (LPTSTR)&pMessage, 0, NULL);

		String message;
#if UNICODE
		message = StringConverter::WStringToString(pMessage);
#else
		message = pMessage;
#endif
		LocalFree((HLOCAL)pMessage);

		// Erase the new line from the message
		if (message.size() >= 2)
		{
			if (message[message.size() - 1] == '\n')
				message[message.size() - 1] = (char)0;
			if (message[message.size() - 2] == '\r')
				message[message.size() - 2] = (char)0;
		}

		return message;
	}

	inline String FormatHResultMessage(HRESULT hResult)
	{
		String message;
		_com_error error(hResult);
#if UNICODE
		message = StringConverter::WStringToString(error.ErrorMessage());
#else
		message = error.ErrorMessage();
#endif
		return message;
	}

	inline String GetLastErrorMessage()
	{
		DWORD error = ::GetLastError();
		return FormatErrorMessage(error);
	}
}
