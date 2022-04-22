#include "IonPCH.h"

#include "Core/Platform/Windows/WindowsCore.h"

#include "Core/File/File.h"

#include "Core/CoreMacros.h"

#ifdef ION_LOG_ENABLED

#define _PRINT_HANDLE_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be accessed before it is physically opened!"), m_FilePath);
#define _PRINT_READ_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be read because the Read access mode was not specified when opening the file!"), m_FilePath);
#define _PRINT_WRITE_ERROR() \
LOG_ERROR(TEXT("File '{0}' cannot be written because the Write access mode was not specified when opening the file!"), m_FilePath);

#else

#define _PRINT_HANDLE_ERROR()
#define _PRINT_READ_ERROR()
#define _PRINT_WRITE_ERROR()

#endif

#define _VERIFY_HANDLE(...) \
if (m_NativeFile.m_FileHandle == INVALID_HANDLE_VALUE) \
{ \
	_PRINT_HANDLE_ERROR(); \
	return __VA_ARGS__; \
}

/* Code inside this macro works only on debug and with DebugLog enabled. */
#define _DEBUG_LOG(x)          DEBUG( if (m_DebugLog) { x; } )
#define _DEBUG_STATIC_LOG(x)   DEBUG( if (s_GlobalDebugLog) { x; } )


namespace Ion
{ 
	bool File::Open_Native() // static
	{
		// Handle errors first
		if (m_NativeFile.m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LOG_ERROR(TEXT("File '{0}' is already open!"), m_FilePath);
			return false;
		}

		// Set Windows flags and options based on internal ones

		DWORD dwDesiredAccess = 0;
		DWORD dwCreationDisposition = OPEN_EXISTING;

		bool bAppend = false;

		if (m_Mode & EFileMode::Read)
		{
			dwDesiredAccess |= GENERIC_READ;
		}

		if (m_Mode & EFileMode::Write)
		{
			dwDesiredAccess |= GENERIC_WRITE;

			if (m_Mode & EFileMode::CreateNew)
			{
				dwCreationDisposition = (m_Mode & EFileMode::Reset) ? CREATE_ALWAYS : OPEN_ALWAYS;
			}
			else if (m_Mode & EFileMode::Reset)
			{
				dwCreationDisposition = TRUNCATE_EXISTING;
			}
			else if (m_Mode & EFileMode::Append)
			{
				// This flag doesn't change anything if the file is empty,
				// which is the case when creating a new file or resetting an existing one.
				// That's why it's ignored until both of the above flags are not set
				bAppend = true;
			}
		}

		m_NativeFile.m_FileHandle = CreateFile(m_FilePath.c_str(), dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (m_NativeFile.m_FileHandle == INVALID_HANDLE_VALUE)
		{
			DWORD lastError = GetLastError();
			if (lastError != ERROR_FILE_NOT_FOUND)
			{
				Windows::PrintLastError(TEXT("File '{0}' cannot be opened!"), m_FilePath);
				return false;
			}

			_DEBUG_LOG(LOG_WARN(TEXT("File '{0}' not found!"), m_FilePath));
		}

		UpdateFileSizeCache();

		if (bAppend)
		{
			// Set the pointer to the end of the file
			SetOffset(m_FileSize);
		}

		_DEBUG_LOG(LOG_TRACE(TEXT("File '{0}' opened."), m_FilePath));
		return true;
	}

	bool File::Delete_Native(const WString& filename) // static
	{
		const wchar* path = filename.c_str();

		if (!DeleteFile(path))
		{
			Windows::PrintLastError(TEXT("Cannot delete file '{0}'!"), path);
			return false;
		}

		_DEBUG_STATIC_LOG(LOG_DEBUG(TEXT("File '{0}' has been deleted."), path));
		return true;
	}

	bool File::Close_Native()
	{
		CloseHandle(m_NativeFile.m_FileHandle);
		_DEBUG_LOG(LOG_TRACE(TEXT("File '{0}' was closed."), m_FilePath));

		//*(size_t*)&m_NativeFile.m_FileHandle = (size_t)INVALID_HANDLE_VALUE;
		m_NativeFile.m_FileHandle = INVALID_HANDLE_VALUE;

		return true;
	}

	bool File::Read_Native(uint8* outBuffer, uint64 count)
	{
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		_VERIFY_HANDLE(false);

		DWORD bytesRead;
		if (!ReadFile(m_NativeFile.m_FileHandle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot read file '{0}'!"), m_FilePath);
			return false;
		}
		m_Offset += bytesRead;

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_FilePath, bytesRead));
		return true;
	}

	bool File::ReadLine_Internal(char* outBuffer, uint64 count, uint64* outReadCount, bool* bOutOverflow)
	{
#pragma warning(disable:6385)
#pragma warning(disable:26451)
		// @TODO: Make the CRLF support more correct with a bigger temporary buffer

		if (bOutOverflow != nullptr)
			*bOutOverflow = false;

		int64 initialOffset = m_Offset;

		DWORD bytesRead;
		if (!ReadFile(m_NativeFile.m_FileHandle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot read file '{0}'!"), m_FilePath);
			return false;
		}

		// Retrieves the file pointer
		m_Offset += bytesRead;

		bool bNewLineFound = false;
		bool bCRLF = false;
		bool bCR = false;
		// This will be the index of the terminating NULL.
		uint32 zeroIndex = bytesRead - 1;
		for (uint32 i = 0; i < bytesRead; ++i)
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
					_DEBUG_LOG(LOG_WARN(TEXT("'{0}': CR was found but the next byte could not be checked! {1} byte buffer was to small."), m_FilePath, count));
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
		if (m_Offset == m_FileSize)
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
		if (!(bNewLineFound || m_Offset == m_FileSize))
		{
			// This is still considered a successful read
			// but it's not a complete one.
			if (bOutOverflow != nullptr)
				*bOutOverflow = true;

			_DEBUG_LOG(LOG_WARN(TEXT("'{0}': File read output buffer overflow! {1} byte buffer was to small."), m_FilePath, count));
		}

		return true;
	}

	bool File::ReadLine_Native(char* outBuffer, uint64 count)
	{
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		_VERIFY_HANDLE(false);

		uint64 readCount = 0;
		bool bResult = ReadLine_Internal(outBuffer, count, &readCount, nullptr);

		_DEBUG_LOG(if (bResult) LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_FilePath, readCount));
		return bResult;
	}

	bool File::ReadLine_Native(String& outStr)
	{
		_VERIFY_HANDLE(false);

		outStr.clear();
		bool bOverflow = false;
		uint64 readCount = 0;
		bool bResult;

		// Start with a 512B buffer
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

		_DEBUG_LOG(if (bResult) LOG_TRACE(TEXT("'{0}': Read {1} bytes."), m_FilePath, readCount));
		return bResult;
	}

	bool File::Write_Native(const uint8* inBuffer, uint64 count)
	{
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		_VERIFY_HANDLE(false);

		DWORD bytesWritten;
		if (!WriteFile(m_NativeFile.m_FileHandle, inBuffer, (DWORD)count, &bytesWritten, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_FilePath);
			return false;
		}
		// Retrieves the file pointer
		m_Offset += bytesWritten;

		// Compare new offset to current file size cache
		// and set the new size based on the result.
		int64 sizeDifference = std::max((int64)0, m_Offset - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': Written {1} bytes."), m_FilePath, bytesWritten));
		return true;
	}

	bool File::WriteLine_Native(const char* inBuffer, uint64 count, ENewLineType newLineType)
	{
#pragma warning(disable:6011)
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");
		ionassert(newLineType != ENewLineType::CRLF || count < std::numeric_limits<DWORD>::max());

		_VERIFY_HANDLE(false);

		bool bCRLF = newLineType == ENewLineType::CRLF;
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
			char newLineChar = newLineType == ENewLineType::LF ? '\n' : '\r';
			tempBuffer[count - 1] = newLineChar;
		}

		ulong bytesWritten;
		if (!WriteFile(m_NativeFile.m_FileHandle, tempBuffer, (DWORD)count + bCRLF, &bytesWritten, NULL))
		{
			Windows::PrintLastError(TEXT("Cannot write file '{0}'!"), m_FilePath);
			return false;
		}

		// Free the heap allocated memory
		_freea(tempBuffer);

		// Retrieves the file pointer
		m_Offset += bytesWritten;

		// Compare new offset to current file size cache
		// and set the new size based on the result.
		// (Faster than actually getting the filesize)
		int64 sizeDifference = std::max((int64)0, m_Offset - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': Written {1} bytes."), m_FilePath, bytesWritten));
		return true;
	}

	bool File::AddOffset_Native(int64 count)
	{
		_VERIFY_HANDLE(false);

		if (!SetFilePointerEx(m_NativeFile.m_FileHandle, *(LARGE_INTEGER*)&count, (LARGE_INTEGER*)&m_Offset, FILE_CURRENT))
		{
			Windows::PrintLastError(TEXT("'{0}': Cannot add file offset!"), m_FilePath);
			return false;
		}
		return true;
	}

	bool File::SetOffset_Native(int64 count)
	{
		_VERIFY_HANDLE(false);

		if (!SetFilePointerEx(m_NativeFile.m_FileHandle, *(LARGE_INTEGER*)&m_Offset, (LARGE_INTEGER*)&m_Offset, FILE_BEGIN))
		{
			Windows::PrintLastError(TEXT("'{0}': Cannot set file offset!"), m_FilePath);
			return false;
		}
		return true;
	}

	int64 File::GetSize() const
	{
		_VERIFY_HANDLE(-1);

		if (m_FileSize == -1)
		{
			GetFileSizeEx(m_NativeFile.m_FileHandle, (LARGE_INTEGER*)&m_FileSize);

			_DEBUG_LOG(LOG_TRACE(TEXT("'{0}': New file size cache = {1} bytes."), m_FilePath, m_FileSize));
		}
		
		return m_FileSize;
	}

	bool File::Exists() const
	{
		return GetFileAttributes(m_FilePath.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	bool File::IsDirectory() const
	{
		return GetFileAttributes(m_FilePath.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
	}

	FilePath File::GetFilePath(EFilePathValidation validation) const
	{
		return FilePath(m_FilePath, validation);
	}

	const ENewLineType File::s_DefaultNewLineType = ENewLineType::CRLF;

	// -----------------------------------------------------
	// FilePath: -------------------------------------------
	// -----------------------------------------------------

	bool FilePath::Make_Native(const wchar* name)
	{
		WString path = m_PathName + name;

		if (!CreateDirectory(path.c_str(), NULL))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_ALREADY_EXISTS)
			{
				LOG_WARN(L"The path \"{0}\" already exists!");
				return false;
			}
			if (dwError == ERROR_PATH_NOT_FOUND)
			{
				LOG_ERROR(L"Cannot make path \"{0}\"!");
				// @TODO: Make this recursive?
				return false;
			}
		}

		return true;
	}

	bool FilePath::Delete_Native()
	{
		return RemoveDirectory(m_PathName.c_str());
	}

	bool FilePath::DeleteForce_Native()
	{
		// @TODO: Remove all the files from the directory first

		return Delete_Native();
	}

	bool FilePath::Exists_Native(const wchar* path)
	{
		return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
	}

	bool FilePath::IsDirectory_Native(const wchar* path)
	{
		return GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY;
	}

	TArray<FileInfo> FilePath::ListFiles_Native() const
	{
		TArray<FileInfo> files;
		WIN32_FIND_DATA ffd;

		WString path = m_PathName + L"/*";

		HANDLE hFound = FindFirstFile(path.c_str(), &ffd);
		if (hFound == INVALID_HANDLE_VALUE)
		{
			return files;
		}

		do
		{
			//wchar fullPath[MaxPathLength + 1];
			//GetFullPathName(ffd.cFileName, MaxPathLength + 1, fullPath, nullptr);

			WString fullPath = m_PathName + L"/" + ffd.cFileName;

			LARGE_INTEGER fileSize;
			fileSize.LowPart = ffd.nFileSizeLow;
			fileSize.HighPart = ffd.nFileSizeHigh;

			bool bDirectory = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

			files.emplace_back(ffd.cFileName, fullPath, fileSize.QuadPart, bDirectory);
		}
		while (FindNextFile(hFound, &ffd));

		return files;
	}
}
