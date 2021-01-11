#include "IonPCH.h"

#include "WindowsFile.h"
#include "Core/Platform/Windows/WindowsCore.h"

#ifdef ION_LOG_ENABLED

#define PRINT_HANDLE_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be accessed before it is physically opened!"), m_Filename);
#define PRINT_READ_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be read because the Read access mode was not specified when opening the file!"), m_Filename);
#define PRINT_WRITE_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be written because the Write access mode was not specified when opening the file!"), m_Filename);

#else

#define PRINT_HANDLE_ERROR()
#define PRINT_READ_ERROR()
#define PRINT_WRITE_ERROR()

#endif

namespace Ion
{
	WindowsFile::WindowsFile()
		: WindowsFile(TEXT(""))
	{ }

	WindowsFile::WindowsFile(const std::wstring& filename) :
		FileBase(filename),
		m_FileHandle(INVALID_HANDLE_VALUE),
		m_Offset({ 0, 0 }),
		m_Mode((IO::FileMode)0)
	{ }

	WindowsFile::~WindowsFile()
	{
		Close();
	}

	bool WindowsFile::SetFilename_Impl(const std::wstring& filename)
	{
		if (IsOpen())
		{
			LOG_WARN(TEXT("Cannot set the file name when the file is open!"));
			return false;
		}
		return true;
	}

	bool WindowsFile::SetType_Impl(IO::FileType type)
	{
		if (IsOpen())
		{
			LOG_WARN(TEXT("Cannot set the file type when the file is open!"));
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
			LOG_ERROR(TEXT("FileMode has to have either Read or Write flag set!"));
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

			LOG_WARN(TEXT("File '{0}' not found!"), m_Filename);

			m_FileHandle = CreateFile(m_Filename.c_str(), dwDesiredAccess, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_FileHandle == INVALID_HANDLE_VALUE)
				goto lError;

			LOG_TRACE(TEXT("File '{0}' created."), m_Filename);
		}

		if (mode & IO::FM_Write && mode & IO::FM_Append)
		{
			// Set the pointer to the end of the file
			llong offset = GetSize();
			SetOffset(offset);
		}

		LOG_TRACE(TEXT("File '{0}' opened."), m_Filename);
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
			LOG_TRACE(TEXT("File '{0}' was closed."), m_Filename);
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
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_Filename);
			return false;
		}
		LOG_INFO(TEXT("File '{0}' was deleted."), m_Filename);
		return true;
	}

	bool WindowsFile::Read(byte* outBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Read))
		{
			PRINT_READ_ERROR();
			return false;
		}

		ulong bytesRead;
		if (!ReadFile(m_FileHandle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot read file '{0}'!"), m_Filename);
			return false;
		}
		// Retrieves the file pointer
		SetFilePointerEx(m_FileHandle, { 0 }, &m_Offset, FILE_CURRENT);

		LOG_DEBUG("Read {0} bytes.", bytesRead);
		return true;
	}

	bool WindowsFile::ReadLine_Internal(char* outBuffer, ullong count, ullong* outReadCount, bool* bOutOverflow)
	{
		// @TODO: Optimise the whole thing

		if (bOutOverflow != nullptr)
			*bOutOverflow = false;

		llong initialOffset = m_Offset.QuadPart;
		llong fileSize = GetSize();
		ullong remainingBufferSize = count;
		ullong localOffset = 0;
		while (true)
		{
			// Read 1KB at first, unless the remaining buffer section is smaller.
			ushort bufferSize = (ushort)std::min(1024ull, remainingBufferSize);
			char* tempBuffer = new char[bufferSize];
			ZeroMemory(tempBuffer, bufferSize);

			DWORD bytesRead;
			if (!ReadFile(m_FileHandle, tempBuffer, (DWORD)bufferSize, &bytesRead, NULL))
			{
				Windows::PrintLastError(TEXT("Cannot read file '{0}'!"), m_Filename);
				return false;
			}

			bool bNewLineFound = false;
			ulong lastIndex = bytesRead - 1;
			for (ushort i = 0; i < bytesRead; ++i)
			{
				if (tempBuffer[i] == '\n')
				{
					// Set new line index to last character to copy
					lastIndex = i;
					bNewLineFound = true;
					break;
				}
			}
			
			// If there's no new line just retreive the pointer.
			// If there is, set the offset to the new line, so the next one
			// can be read in another function call.
			if (bNewLineFound)
			{
				SetOffset(initialOffset + localOffset + lastIndex + 1);
			}
			else
			{
				// Retrieves the file pointer
				SetFilePointerEx(m_FileHandle, { 0 }, &m_Offset, FILE_CURRENT);
			}

			// End of file works like a new line character in this situation
			memcpy_s(outBuffer + localOffset, remainingBufferSize, tempBuffer, lastIndex + 1);
			delete[] tempBuffer;

			// Edge case:
			// Output buffer cannot fit the NULL character at the end.
			// In this situation we have to treat it
			// kind of like an overflow.
			if (remainingBufferSize - (lastIndex + 1) == 0)
			{
				outBuffer[count - 1] = '\0';
				AddOffset(-1);
				// This localOffset variable is also treated as "all bytes read"
				// so increment it only so much that the new line
				// character is not considered.
				localOffset += lastIndex;
			}
			// Otherwise just add the NULL character at the end.
			else
			{
				outBuffer[localOffset + lastIndex + 1] = '\0';
				localOffset += lastIndex + 1;
			}

			// If there's no new line and it's not the end of file keep going with a new offset.
			if (bNewLineFound || m_Offset.QuadPart == fileSize)
				break;

			remainingBufferSize -= lastIndex + 1;

			// Check so there's no buffer overflow
			// This is still considered a successful read
			// but it's not a complete one.
			if (!remainingBufferSize)
			{
				if (bOutOverflow != nullptr)
					*bOutOverflow = true;
				//LOG_WARN(TEXT("File read output buffer overflow! {1} byte buffer was to small. ('{0}')"), m_Filename, count);
				break;
			}
		}
		if (outReadCount != nullptr)
			*outReadCount = localOffset;

		//LOG_DEBUG("Read {0} bytes.", localOffset);
		return true;
	}

	bool WindowsFile::ReadLine(char* outBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Read))
		{
			PRINT_READ_ERROR();
			return false;
		}

		return ReadLine_Internal(outBuffer, count, nullptr, nullptr);
	}

	bool WindowsFile::ReadLine(std::string& outStr)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Read))
		{
			PRINT_READ_ERROR();
			return false;
		}

		outStr = "";
		bool bOverflow = false;
		// Start with 1KB buffer
		const uint bufferSize = 1024;
		char* tempBuffer = new char[bufferSize];
		// Call the internal ReadLine function until we hit the new line character
		do
		{
			ZeroMemory(tempBuffer, bufferSize);
			ReadLine_Internal(tempBuffer, bufferSize, nullptr, &bOverflow);
			outStr += tempBuffer;
		}
		while (bOverflow);
		delete[] tempBuffer;

		//LOG_DEBUG("Read {0} bytes.", localOffset);
		return true;
	}

	bool WindowsFile::Write(const byte* inBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Write))
		{
			PRINT_WRITE_ERROR();
			return false;
		}

		ulong bytesWritten;
		if (!WriteFile(m_FileHandle, inBuffer, (DWORD)count, &bytesWritten, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_Filename);
			return false;
		}
		// Retrieves the file pointer
		SetFilePointerEx(m_FileHandle, { 0 }, &m_Offset, FILE_CURRENT);

		LOG_DEBUG("Written {0} bytes.", bytesWritten);
		return true;
	}

	bool WindowsFile::WriteLine(const char* inBuffer, ullong count)
	{
		// Handle internal errors
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}
		if (!(m_Mode & IO::FM_Write))
		{
			PRINT_WRITE_ERROR();
			return false;
		}

		// Copy the inBuffer and set the last character to NewLine
		char* tempBuffer = new char[count + 1];
		memcpy_s(tempBuffer, count, inBuffer, count);
		tempBuffer[count] = '\n';

		ulong bytesWritten;
		if (!WriteFile(m_FileHandle, tempBuffer, (DWORD)(count + 1), &bytesWritten, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_Filename);
			return false;
		}
		delete[] tempBuffer;

		// Retrieves the file pointer
		SetFilePointerEx(m_FileHandle, { 0 }, &m_Offset, FILE_CURRENT);

		LOG_DEBUG("Written {0} bytes.", bytesWritten);
		return true;
	}

	bool WindowsFile::WriteLine(const std::string& inStr)
	{
		return WriteLine(inStr.c_str(), inStr.size());
	}

	bool WindowsFile::AddOffset(llong count)
	{
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}

		LARGE_INTEGER _count;
		_count.QuadPart = count;
		if (!SetFilePointerEx(m_FileHandle, _count, &m_Offset, FILE_CURRENT))
		{
			Windows::PrintLastError(TEXT("Cannot add file offset! ('{0}')"), m_Filename);
			return false;
		}
		return true;
	}

	bool WindowsFile::SetOffset(llong count)
	{
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return false;
		}

		LARGE_INTEGER _count;
		_count.QuadPart = count;
		if (!SetFilePointerEx(m_FileHandle, _count, &m_Offset, FILE_BEGIN))
		{
			Windows::PrintLastError(TEXT("Cannot set file offset! ('{0}')"), m_Filename);
			return false;
		}
		return true;
	}
	
	llong WindowsFile::GetSize() const
	{
		if (m_FileHandle == INVALID_HANDLE_VALUE)
		{
			PRINT_HANDLE_ERROR();
			return -1ll;
		}

		LARGE_INTEGER fileSize;
		GetFileSizeEx(m_FileHandle, &fileSize);
		return fileSize.QuadPart;
	}

	bool WindowsFile::Exists() const
	{
		DWORD attributes = GetFileAttributes(m_Filename.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES &&
			!(attributes & FILE_ATTRIBUTE_DIRECTORY);
	}
}
