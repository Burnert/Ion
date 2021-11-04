#include "IonPCH.h"

#include "WindowsFileOld.h"
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

/* Code inside this macro works only on debug and with DebugLog enabled. */
#define _DEBUG_LOG(x)          DEBUG( if (m_DebugLog) { x; } )
#define _DEBUG_STATIC_LOG(x)   DEBUG( if (s_GlobalDebugLog) { x; } )


namespace Ion
{
	FileOld* FileOld::Create()
	{
		return new WindowsFileOld();
	}

	FileOld* FileOld::Create(const WString& filename)
	{
		return new WindowsFileOld(filename);
	}

	WindowsFileOld::WindowsFileOld()
		: WindowsFileOld(TEXT(""))
	{ }

	WindowsFileOld::WindowsFileOld(const WString& filename) :
		FileOld(filename),
		m_FileHandle(INVALID_HANDLE_VALUE),
		m_Offset({ 0, 0 })
	{ }

	WindowsFileOld::~WindowsFileOld()
	{
		Close();
	}

	bool WindowsFileOld::SetFilename_Impl(const WString& filename)
	{
		if (IsOpen())
		{
			LOG_WARN(TEXT("'{0}': Cannot set the file name when the file is open!"));
			return false;
		}
		return true;
	}

	bool WindowsFileOld::SetType_Impl(IO::EFileType type)
	{
		if (IsOpen())
		{
			LOG_WARN(TEXT("'{0}': Cannot set the file type when the file is open!"));
			return false;
		}
		return true;
	}

	bool WindowsFileOld::Open(uint8 mode)
	{
		// This makes sure the filename is not blank before opening the file.
		// If it is, then clearly something went wrong.
		ionassert(IsFilenameValid());

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

			_DEBUG_LOG( LOG_WARN(TEXT("File '{0}' not found!"), m_Filename) );

			m_FileHandle = CreateFile(m_Filename.c_str(), dwDesiredAccess, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_FileHandle == INVALID_HANDLE_VALUE)
				goto lError;

			_DEBUG_LOG( LOG_DEBUG(TEXT("File '{0}' created."), m_Filename) );
		}

		UpdateFileSizeCache();

		if (mode & IO::FM_Write && mode & IO::FM_Append)
		{
			// Set the pointer to the end of the file
			int64 offset = m_FileSize;
			SetOffset(offset);
		}

		_DEBUG_LOG( LOG_TRACE(TEXT("File '{0}' opened."), m_Filename) );
		return true;

		// Other error
	lError:
		Windows::PrintLastError(TEXT("File '{0}' cannot be opened!"), m_Filename);
		return false;
	}

	void WindowsFileOld::Close()
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			_DEBUG_LOG( LOG_TRACE(TEXT("File '{0}' was closed."), m_Filename) );
			CloseHandle(m_FileHandle);
			m_FileHandle = INVALID_HANDLE_VALUE;
		}
	}

	bool WindowsFileOld::Delete()
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

		_DEBUG_LOG( LOG_DEBUG(TEXT("File '{0}' was deleted."), m_Filename) );
		return true;
	}

	bool WindowsFileOld::Read(uint8* outBuffer, uint64 count)
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

		_DEBUG_LOG( LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_Filename, bytesRead) );
		return true;
	}

	bool WindowsFileOld::Read(String& outStr)
	{
		int64 count = GetSize();
		char* buffer = new char[count + 1];
		buffer[count] = (char)0;

		bool bResult = Read((uint8*)buffer, count);
		if (bResult)
		{
			// @TODO: Why would I want to copy this whole buffer instead of just moving the pointer into the string?
			outStr.assign(buffer);
		}
		delete[] buffer;

		return bResult;
	}

	bool WindowsFileOld::ReadLine_Internal(char* outBuffer, uint64 count, uint64* outReadCount, bool* bOutOverflow)
	{
		// @TODO: Make the CRLF support more correct with a bigger temporary buffer

		if (bOutOverflow != nullptr)
			*bOutOverflow = false;

		int64 initialOffset = m_Offset.QuadPart;

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
		for (uint16 i = 0; i < bytesRead; ++i)
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
					_DEBUG_LOG( LOG_WARN(TEXT("'{0}': CR was found but the next byte could not be checked! {1} byte buffer was to small."), m_Filename, count) );
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

			_DEBUG_LOG( LOG_WARN(TEXT("'{0}': File read output buffer overflow! {1} byte buffer was to small."), m_Filename, count) );
		}

		return true;
	}

	bool WindowsFileOld::ReadLine(char* outBuffer, uint64 count)
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

		uint64 readCount = 0;
		bool bResult = ReadLine_Internal(outBuffer, count, &readCount, nullptr);

		_DEBUG_LOG( if (bResult) LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_Filename, readCount) );
		return bResult;
	}

	bool WindowsFileOld::ReadLine(String& outStr)
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
		uint64 readCount = 0;
		bool bResult;
		// Start with 512B buffer
		const uint32 bufferSize = 512;
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

		_DEBUG_LOG( if (bResult) LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_Filename, readCount) );
		return bResult;
	}

	bool WindowsFileOld::Write(const uint8* inBuffer, uint64 count)
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
		int64 sizeDifference = std::max(0ll, m_Offset.QuadPart - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		_DEBUG_LOG( LOG_TRACE(TEXT("'{0}': Written {1} bytes."), m_Filename, bytesWritten) );
		return true;
	}

	bool WindowsFileOld::Write(const String& inStr)
	{
		return Write((const uint8*)inStr.c_str(), inStr.size());
	}

	bool WindowsFileOld::WriteLine(const char* inBuffer, uint64 count)
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
		int64 sizeDifference = std::max(0ll, m_Offset.QuadPart - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		_DEBUG_LOG( LOG_TRACE(TEXT("'{0}': Written {1} bytes."), m_Filename, bytesWritten) );
		return true;
	}

	bool WindowsFileOld::WriteLine(const String& inStr)
	{
		return WriteLine(inStr.c_str(), inStr.size() + 1);
	}

	bool WindowsFileOld::AddOffset(int64 count)
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

	bool WindowsFileOld::SetOffset(int64 count)
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

	int64 WindowsFileOld::GetOffset() const
	{
		return m_Offset.QuadPart; 
	}
	
	int64 WindowsFileOld::GetSize() const
	{
		// @TODO: Make this function work even without a need to open the file

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

		_DEBUG_LOG( LOG_TRACE(TEXT("'{0}': New file size cache = {1} bytes."), m_Filename, m_FileSize) );
		return fileSize.QuadPart;
	}

	WString WindowsFileOld::GetExtension() const
	{
		uint64 dotIndex = m_Filename.find_last_of(L'.');
		WString filename = m_Filename.substr(dotIndex + 1, (size_t)-1);
		std::transform(filename.begin(), filename.end(), filename.begin(), [](wchar ch) { return std::tolower(ch); });
		return filename;
	}

	bool WindowsFileOld::IsDirectory() const
	{
		DWORD attributes = GetFileAttributes(m_Filename.c_str());
		return attributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	TArray<FileInfo> WindowsFileOld::GetFilesInDirectory() const
	{
		ionassert(m_Filename != L"");
		ionassert(IsDirectory());

		TArray<FileInfo> files;
		WIN32_FIND_DATA ffd;

		WString directory = m_Filename + L"\\*";

		HANDLE hFound = FindFirstFile(directory.c_str(), &ffd);
		if (hFound == INVALID_HANDLE_VALUE)
		{
			return files;
		}

		do
		{
			wchar fullPath[MAX_PATH + 1];
			GetFullPathName(ffd.cFileName, MAX_PATH + 1, fullPath, nullptr);

			LARGE_INTEGER fileSize;
			fileSize.LowPart = ffd.nFileSizeLow;
			fileSize.HighPart = ffd.nFileSizeHigh;

			bool bDirectory = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

			files.emplace_back(FileInfo { ffd.cFileName, fullPath, fileSize.QuadPart, bDirectory });
		}
		while (FindNextFile(hFound, &ffd));

		return files;
	}

	WString WindowsFileOld::FindInDirectoryRecursive(const WString& filename) const
	{
		// @TODO: Implement this

		return WString();
	}

	bool WindowsFileOld::IsOpen() const
	{ 
		return m_FileHandle != INVALID_HANDLE_VALUE;
	}

	bool WindowsFileOld::Exists() const
	{
		DWORD attributes = GetFileAttributes(m_Filename.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES;
	}

	bool WindowsFileOld::EndOfFile() const
	{
		return m_Offset.QuadPart >= GetSize();
	}
}
