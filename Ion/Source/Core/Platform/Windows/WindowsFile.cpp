#include "IonPCH.h"

#include "WindowsFile.h"
#include "Core/CoreMacros.h"
#include "Core/Platform/Windows/WindowsUtility.h"

#ifdef ION_LOG_ENABLED

#define _PRINT_HANDLE_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be accessed before it is physically opened!"), m_Filename);
#define _PRINT_READ_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be read because the Read access mode was not specified when opening the file!"), m_Filename);
#define _PRINT_WRITE_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be written because the Write access mode was not specified when opening the file!"), m_Filename);

#else

#define _PRINT_HANDLE_ERROR()
#define _PRINT_READ_ERROR()
#define _PRINT_WRITE_ERROR()

#endif

#ifdef ION_DEBUG
	/* Code inside this macro works only on debug and with DebugLog enabled. */
	#define _DEBUG_LOG(x)   if (m_DebugLog) { x; }

#else
	/* Code inside this macro works only on debug and with DebugLog enabled. */
	#define _DEBUG_LOG(x)

#endif

namespace Ion
{
	WindowsFile::WindowsFile()
		: WindowsFile(TEXT(""))
	{ }

	WindowsFile::WindowsFile(const std::wstring& filename) :
		FileBase(filename),
		m_FileHandle(INVALID_HANDLE_VALUE),
		m_Offset({ 0, 0 })
	{ }

	WindowsFile::~WindowsFile()
	{
		Close();
	}

	bool WindowsFile::SetFilename_Impl(const std::wstring& filename)
	{
		if (IsOpen())
		{
			LOG_WARN(TEXT("'{0}': Cannot set the file name when the file is open!"));
			return false;
		}
		return true;
	}

	bool WindowsFile::SetType_Impl(IO::FileType type)
	{
		if (IsOpen())
		{
			LOG_WARN(TEXT("'{0}': Cannot set the file type when the file is open!"));
			return false;
		}
		return true;
	}

	bool WindowsFile::Open(byte mode)
	{
		// This makes sure the filename is not blank before opening the file.
		// If it is, then clearly something went wrong.
		ASSERT(IsFilenameValid())

		// Handle errors first
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LOG_ERROR(TEXT("File '{0}' is already open!"), m_Filename);
			return false;
		}
		if (!(mode & (IO::FM_Read | IO::FM_Write)))
		{
			LOG_ERROR(TEXT("'{0}': FileMode has to have either Read or Write flag set!"));
			return false;
		}

		// Set Windows flags and options based on internal ones

		DWORD dwDesiredAccess = 0;
		DWORD dwCreationDisposition = OPEN_EXISTING;
		if (mode & IO::FM_Read)
			dwDesiredAccess |= GENERIC_READ;
		if (mode & IO::FM_Write)
			dwDesiredAccess |= GENERIC_WRITE;
		if (mode & IO::FM_Reset)
			dwCreationDisposition = TRUNCATE_EXISTING;

		m_Mode = mode;
		m_FileHandle = CreateFile(m_Filename.c_str(), dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			DWORD lastError = GetLastError();
			if (lastError != ERROR_FILE_NOT_FOUND)
				goto lError;

			_DEBUG_LOG(LOG_WARN(TEXT("File '{0}' not found!"), m_Filename));

			m_FileHandle = CreateFile(m_Filename.c_str(), dwDesiredAccess, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_FileHandle == INVALID_HANDLE_VALUE)
				goto lError;

			_DEBUG_LOG(LOG_DEBUG(TEXT("File '{0}' created."), m_Filename));
		}

		UpdateFileSizeCache();

		if (mode & IO::FM_Write && mode & IO::FM_Append)
		{
			// Set the pointer to the end of the file
			llong offset = m_FileSize;
			SetOffset(offset);
		}

		_DEBUG_LOG(LOG_TRACE(TEXT("File '{0}' opened."), m_Filename));
		return true;

		// Other error
	lError:
		Windows::PrintLastError(TEXT("File '{0}' cannot be opened!"), m_Filename);
		return false;
	}

	void WindowsFile::Close()
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			_DEBUG_LOG(LOG_TRACE(TEXT("File '{0}' was closed."), m_Filename));
			CloseHandle(m_FileHandle);
			m_FileHandle = INVALID_HANDLE_VALUE;
		}
	}

	bool WindowsFile::Delete()
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LOG_ERROR(TEXT("Cannot delete '{0}' before it is closed!"), m_Filename);
			return false;
		}

		if (!DeleteFile(m_Filename.c_str()))
		{
			Windows::PrintLastError(TEXT("Cannot delete file '{0}'!"), m_Filename);
			return false;
		}

		_DEBUG_LOG(LOG_DEBUG(TEXT("File '{0}' was deleted."), m_Filename));
		return true;
	}

	bool WindowsFile::Read(byte* outBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Read))
		{
			_PRINT_READ_ERROR();
			return false;
		}

		ulong bytesRead;
		if (!ReadFile(m_FileHandle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot read file '{0}'!"), m_Filename);
			return false;
		}
		// Retrieves the file pointer
		m_Offset.QuadPart += bytesRead;

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_Filename, bytesRead));
		return true;
	}

	bool WindowsFile::ReadLine_Internal(char* outBuffer, ullong count, ullong* outReadCount, bool* bOutOverflow)
	{
		// @TODO: Make the CRLF support more correct with a bigger temporary buffer

		if (bOutOverflow != nullptr)
			*bOutOverflow = false;

		llong initialOffset = m_Offset.QuadPart;

		DWORD bytesRead;
		if (!ReadFile(m_FileHandle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot read file '{0}'!"), m_Filename);
			return false;
		}

		// Retrieves the file pointer
		m_Offset.QuadPart += bytesRead;

		bool bNewLineFound = false;
		bool bCRLF = false;
		bool bCR = false;
		// This will be the index of the terminating NULL.
		ulong zeroIndex = bytesRead - 1;
		for (ushort i = 0; i < bytesRead; ++i)
		{
			// Windows NewLine is CRLF - 0D0A, so it has to be handled appropriately.
			// If this is the last byte it should be treated as an overflow, because
			// the next byte cannot be checked.
			if (outBuffer[i] == '\r')
			{
				if (i < bytesRead - 1)
				{
					if (outBuffer[i + 1] == '\n')
					{
						bCRLF = true;
					}
					else
					{
						// If it's just CR at the end of the line
						// it is still valid, but has to be handled
						// like an LF type new line
						bCR = true;
					}
				}
				else
				{
					_DEBUG_LOG(LOG_WARN(TEXT("'{0}': CR was found but the next byte could not be checked! {1} byte buffer was to small."), m_Filename, count));
					// Set the offset back to the CR character
					// so it can be interpreted later and then exit.
					SetOffset(initialOffset + i);
					break;
				}
			}

			// The next step is almost the same for every new line character
			if (outBuffer[i] == '\n' || bCRLF || bCR)
			{
				// Make the last character before new line index
				// the last character to copy by inserting zero
				// at the new line index
				zeroIndex = i;
				bNewLineFound = true;

				// Set new file offset to the first character
				// in the next line
				// If the new line is CRLF it will be 2 characters forward.
				SetOffset(initialOffset + zeroIndex + 1 + bCRLF);
				break;
			}
		}

		// If it is the end of file it has to be treated as a new line
		// so it doesn't cut the last character
		if (m_Offset.QuadPart == m_FileSize)
		{
			// The last character cannot be written if the buffer is too small to fit the terminating zero.
			if (zeroIndex + 1 < count)
			{
				zeroIndex++;
			}
		}

		// Fill zeros from the end of the line to the end of the buffer
		// If the new line was not found it just sets the last byte to 0
		ZeroMemory(outBuffer + zeroIndex, count - zeroIndex);

		if (outReadCount != nullptr)
			*outReadCount = zeroIndex;

		// If it's the end of the file treat it as if new line was found.
		// No overflow in this case.
		if (!(bNewLineFound || m_Offset.QuadPart == m_FileSize))
		{
			// This is still considered a successful read
			// but it's not a complete one.
			if (bOutOverflow != nullptr)
				*bOutOverflow = true;

			_DEBUG_LOG(LOG_WARN(TEXT("'{0}': File read output buffer overflow! {1} byte buffer was to small."), m_Filename, count));
		}

		return true;
	}

	bool WindowsFile::ReadLine(char* outBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Read))
		{
			_PRINT_READ_ERROR();
			return false;
		}

		ullong readCount = 0;
		bool bResult = ReadLine_Internal(outBuffer, count, &readCount, nullptr);

		_DEBUG_LOG(if (bResult) LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_Filename, readCount));
		return bResult;
	}

	bool WindowsFile::ReadLine(std::string& outStr)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Read))
		{
			_PRINT_READ_ERROR();
			return false;
		}

		outStr = "";
		bool bOverflow = false;
		ullong readCount = 0;
		bool bResult;
		// Start with 512B buffer
		const uint bufferSize = 512;
		char tempBuffer[bufferSize];
		// Call the internal ReadLine function until we hit the new line character
		// Should happen instantly unless the line is huge
		do
		{
			bResult = ReadLine_Internal(tempBuffer, bufferSize, &readCount, &bOverflow);
			if (!bResult)
				break;

			outStr += tempBuffer;
		}
		while (bOverflow);

		_DEBUG_LOG(if (bResult) LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_Filename, readCount));
		return bResult;
	}

	bool WindowsFile::Write(const byte* inBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Write))
		{
			_PRINT_WRITE_ERROR();
			return false;
		}

		ulong bytesWritten;
		if (!WriteFile(m_FileHandle, inBuffer, (DWORD)count, &bytesWritten, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_Filename);
			return false;
		}
		// Retrieves the file pointer
		m_Offset.QuadPart += bytesWritten;

		// Compare new offset to current file size cache
		// and set the new size based on the result.
		llong sizeDifference = std::max(0ll, m_Offset.QuadPart - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': Written {1} bytes."), m_Filename, bytesWritten));
		return true;
	}

	bool WindowsFile::WriteLine(const char* inBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Write))
		{
			_PRINT_WRITE_ERROR();
			return false;
		}

		bool bCRLF = WriteNewLineType == IO::NLT_CRLF;
		// Allocate on stack if the buffer is small
		char* tempBuffer = (char*)_malloca(count + bCRLF);
		// Copy the inBuffer and set the last character to NewLine instead of NULL
		memcpy_s(tempBuffer, count, inBuffer, count);
		if (bCRLF)
		{
			char crlf[2] = { '\r', '\n' };
			memcpy(tempBuffer + count - 1, crlf, 2);
		}
		else
		{
			char newLineChar = WriteNewLineType == IO::NLT_LF ? '\n' : '\r';
			tempBuffer[count - 1] = newLineChar;
		}

		ulong bytesWritten;
		if (!WriteFile(m_FileHandle, tempBuffer, (DWORD)count + bCRLF, &bytesWritten, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_Filename);
			return false;
		}

		// Free the heap allocated memory
		_freea(tempBuffer);

		// Retrieves the file pointer
		m_Offset.QuadPart += bytesWritten;

		// Compare new offset to current file size cache
		// and set the new size based on the result.
		// (Faster than actually getting the filesize)
		llong sizeDifference = std::max(0ll, m_Offset.QuadPart - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': Written {1} bytes."), m_Filename, bytesWritten));
		return true;
	}

	bool WindowsFile::WriteLine(const std::string& inStr)
	{
		return WriteLine(inStr.c_str(), inStr.size() + 1);
	}

	bool WindowsFile::AddOffset(llong count)
	{
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}

		LARGE_INTEGER _count;
		_count.QuadPart = count;
		if (!SetFilePointerEx(m_FileHandle, _count, &m_Offset, FILE_CURRENT))
		{
			Windows::PrintLastError(TEXT("'{0}': Cannot add file offset!"), m_Filename);
			return false;
		}
		return true;
	}

	bool WindowsFile::SetOffset(llong count)
	{
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return false;
		}

		LARGE_INTEGER _count;
		_count.QuadPart = count;
		if (!SetFilePointerEx(m_FileHandle, _count, &m_Offset, FILE_BEGIN))
		{
			Windows::PrintLastError(TEXT("'{0}': Cannot set file offset!"), m_Filename);
			return false;
		}
		return true;
	}
	
	llong WindowsFile::GetSize() const
	{
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			_PRINT_HANDLE_ERROR();
			return -1ll;
		}

		if (m_FileSize != -1)
		{
			return m_FileSize;
		}

		LARGE_INTEGER fileSize;
		GetFileSizeEx(m_FileHandle, &fileSize);
		m_FileSize = fileSize.QuadPart;

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': New file size cache = {1} bytes."), m_Filename, m_FileSize));
		return fileSize.QuadPart;
	}

	bool WindowsFile::Exists() const
	{
		DWORD attributes = GetFileAttributes(m_Filename.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES &&
			!(attributes & FILE_ATTRIBUTE_DIRECTORY);
	}
}
