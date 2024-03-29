#include "Core/CorePCH.h"

#include "Core/Base.h"
#include "Core/File/File.h"
#include "WindowsCore.h"

#pragma warning(disable:6011)
#pragma warning(disable:6385)
#pragma warning(disable:26451)

#define Handle GetNative(this)

namespace Ion
{
	REGISTER_DEBUG_LOGGER(WindowsFileLogger, "Platform::Windows::File", ELoggerFlags::None, ELogLevel::Warn);

	static HANDLE& GetNative(File* file)
	{
		return file->m_NativePointer;
	}

	static const HANDLE& GetNative(const File* file)
	{
		return file->m_NativePointer;
	}

	Result<void, IOError, FileNotFoundError> File::Open_Native() // static
	{
		// Handle errors first
		ionassert(!m_bOpen, "The file \"{}\" is already open.", m_FilePath.ToString());
		ionassert(Handle == INVALID_HANDLE_VALUE);

		// Set Windows flags and options based on internal ones

		DWORD dwDesiredAccess = 0;
		DWORD dwCreationDisposition = OPEN_EXISTING;

		bool bAppend = false;

		if (m_Mode & EFileMode::Read)
			dwDesiredAccess |= GENERIC_READ;

		if (m_Mode & EFileMode::Write)
		{
			dwDesiredAccess |= GENERIC_WRITE;

			if (m_Mode & EFileMode::CreateNew)
				dwCreationDisposition = (m_Mode & EFileMode::Reset) ? CREATE_ALWAYS : OPEN_ALWAYS;
			else if (m_Mode & EFileMode::Reset)
				dwCreationDisposition = TRUNCATE_EXISTING;
			else if (m_Mode & EFileMode::Append)
			{
				// This flag doesn't change anything if the file is empty,
				// which is the case when creating a new file or resetting an existing one.
				// That's why it's ignored until both of the above flags are not set
				bAppend = true;
			}
		}

		Handle = CreateFile(m_FilePath.ToWString().c_str(), dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (Handle == INVALID_HANDLE_VALUE)
		{
			DWORD error = GetLastError();
			if (error != ERROR_FILE_NOT_FOUND)
			{
				ionthrow(IOError, "File \"{}\" cannot be opened.\n{}", m_FilePath.ToString(), Windows::FormatErrorMessage(error));
			}

			ionthrow(FileNotFoundError, "File \"{}\" not found.", m_FilePath.ToString());
		}

		m_bOpen = true;

		UpdateFileSizeCache();

		if (bAppend)
		{
			// Set the pointer to the end of the file
			SetOffset(m_FileSize);
		}

		return Ok();
	}

	Result<void, IOError, FileNotFoundError> File::Delete_Native() // static
	{
		ionassert(!m_bOpen, "The file needs to be closed before being deleted.");
		ionassert(!m_FilePath.IsEmpty());

		if (!DeleteFile(m_FilePath.ToWString().c_str()))
		{
			DWORD error = ::GetLastError();
			if (error != ERROR_FILE_NOT_FOUND)
			{
				ionthrow(IOError, "Cannot delete file \"{}\".\n{}", m_FilePath.ToString(), Windows::FormatErrorMessage(error));
			}

			ionthrow(FileNotFoundError, "File \"{}\" not found.", m_FilePath.ToString());
		}

		return Ok();
	}

	void File::Close_Native()
	{
		CloseHandle(Handle);
		Handle = INVALID_HANDLE_VALUE;
	}

	Result<void, IOError> File::Read_Native(uint8* outBuffer, uint64 count)
	{
		ionassert(outBuffer);
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		DWORD bytesRead;
		if (!ReadFile(Handle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			ionthrow(IOError, "Cannot read file \"{}\".\n{}", m_FilePath.ToString(), Windows::GetLastErrorMessage());
		}
		m_Offset += bytesRead;

		return Ok();
	}

	Result<void, IOError> File::ReadLine_Internal(char* outBuffer, uint64 count, uint64* outReadCount, bool* bOutOverflow)
	{
		ionassert(outBuffer);
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		// @TODO: Make the CRLF support more correct with a bigger temporary buffer

		if (bOutOverflow != nullptr)
			*bOutOverflow = false;

		int64 initialOffset = m_Offset;

		DWORD bytesRead;
		if (!ReadFile(Handle, outBuffer, (DWORD)count, &bytesRead, NULL))
		{
			ionthrow(IOError, "Cannot read file \"{}\".\n{}", m_FilePath.ToString(), Windows::GetLastErrorMessage());
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
					WindowsFileLogger.Debug("(\"{}\" -> ReadLine_Internal) CR was found but the next byte could not be checked. {}B buffer is too small.", m_FilePath.ToString(), count);
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

			WindowsFileLogger.Debug("(\"{}\" -> ReadLine_Internal) File read output buffer overflow. {} byte buffer was to small.", m_FilePath.ToString(), count);
		}

		return Ok();
	}

	Result<void, IOError> File::ReadLine_Native(char* outBuffer, uint64 count)
	{
		ionassert(outBuffer);
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		uint64 readCount = 0;
		return ReadLine_Internal(outBuffer, count, &readCount, nullptr);
	}

	Result<String, IOError> File::ReadLine_Native()
	{
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);

		String line;
		uint64 readCount = 0;
		bool bOverflow = false;

		// Start with a 512B buffer
		constexpr uint32 bufferSize = 512;
		char tempBuffer[bufferSize];

		// Call the internal ReadLine function until we hit the new line character
		// Should happen instantly unless the line is huge
		do
		{
			fwdthrowall(ReadLine_Internal(tempBuffer, bufferSize, &readCount, &bOverflow));
			line += tempBuffer;
		}
		while (bOverflow);

		return line;
	}

	Result<void, IOError> File::Write_Native(const uint8* inBuffer, uint64 count)
	{
		ionassert(inBuffer);
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");

		DWORD bytesWritten;
		if (!WriteFile(Handle, inBuffer, (DWORD)count, &bytesWritten, NULL))
		{
			ionthrow(IOError, "Cannot write file \"{}\".\n{}", m_FilePath.ToString(), Windows::GetLastErrorMessage());
		}
		// Retrieves the file pointer
		m_Offset += bytesWritten;

		// Compare new offset to current file size cache
		// and set the new size based on the result.
		int64 sizeDifference = std::max((int64)0, m_Offset - m_FileSize);
		UpdateFileSizeCache(m_FileSize + sizeDifference);

		return Ok();
	}

	Result<void, IOError> File::WriteLine_Native(const char* inBuffer, uint64 count, ENewLineType newLineType)
	{
		ionassert(inBuffer);
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);
		ionassert(count <= std::numeric_limits<DWORD>::max(), "Count must fit in a DWORD type.");
		ionassert(newLineType != ENewLineType::CRLF || count < std::numeric_limits<DWORD>::max());

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

		uint32 bytesWritten;
		if (!WriteFile(Handle, tempBuffer, (DWORD)count + bCRLF, (LPDWORD)&bytesWritten, NULL))
		{
			ionthrow(IOError, "Cannot write file \"{}\".\n{}", m_FilePath.ToString(), Windows::GetLastErrorMessage());
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

		return Ok();
	}

	Result<void, IOError> File::AddOffset_Native(int64 count)
	{
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);

		if (!SetFilePointerEx(Handle, *(LARGE_INTEGER*)&count, (LARGE_INTEGER*)&m_Offset, FILE_CURRENT))
		{
			ionthrow(IOError, "Cannot add file offset in file \"{}\".\n{}", m_FilePath.ToString(), Windows::GetLastErrorMessage());
		}
		return Ok();
	}

	Result<void, IOError> File::SetOffset_Native(int64 count)
	{
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);

		if (!SetFilePointerEx(Handle, *(LARGE_INTEGER*)&m_Offset, (LARGE_INTEGER*)&m_Offset, FILE_BEGIN))
		{
			ionthrow(IOError, "Cannot set file offset in file \"{}\".\n{}", m_FilePath.ToString(), Windows::GetLastErrorMessage());
		}
		return Ok();
	}

	void File::SetNativePointer_Native()
	{
		Handle = INVALID_HANDLE_VALUE;
	}

	int64 File::GetSize() const
	{
		ionassert(m_bOpen);
		ionassert(Handle != INVALID_HANDLE_VALUE);

		if (m_FileSize == -1)
		{
			GetFileSizeEx(Handle, (LARGE_INTEGER*)&m_FileSize);

			WindowsFileLogger.Trace("(\"{}\" -> GetSize) New file size cache = {} bytes.", m_FilePath.ToString(), m_FileSize);
		}
		
		return m_FileSize;
	}

	const ENewLineType File::s_DefaultNewLineType = ENewLineType::CRLF;

	// -----------------------------------------------------
	// FilePath: -------------------------------------------
	// -----------------------------------------------------

	bool FilePath::MkDir_Native(const wchar* name)
	{
		WString path = m_PathName.empty() ? name : StringConverter::StringToWString(m_PathName) + L"/" + name;

		if (!CreateDirectory(path.c_str(), NULL))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_ALREADY_EXISTS)
			{
				FileLogger.Warn(L"The path \"{}\" already exists!", path);
				return false;
			}
			if (dwError == ERROR_PATH_NOT_FOUND)
			{
				FileLogger.Error(L"Cannot make path \"{}\"!", path);
				// @TODO: Make this recursive?
				return false;
			}
		}

		return true;
	}

	bool FilePath::Delete_Native() const
	{
		return RemoveDirectory(StringConverter::StringToWString(m_PathName).c_str());
	}

	bool FilePath::DeleteForce_Native() const
	{
		// @TODO: Remove all the files from the directory first

		return Delete_Native();
	}

	TArray<FileInfo> FilePath::ListFiles_Native() const
	{
		TArray<FileInfo> files;
		WIN32_FIND_DATA ffd;

		String path = m_PathName + "/*";

		HANDLE hFound = FindFirstFile(StringConverter::StringToWString(path).c_str(), &ffd);
		if (hFound == INVALID_HANDLE_VALUE)
		{
			return files;
		}

		do
		{
			String fileName = StringConverter::WStringToString(ffd.cFileName);
			String fullPath = m_PathName + "/" + fileName;

			LARGE_INTEGER fileSize;
			fileSize.LowPart = ffd.nFileSizeLow;
			fileSize.HighPart = ffd.nFileSizeHigh;

			bool bDirectory = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

			files.emplace_back(fileName, fullPath, fileSize.QuadPart, bDirectory);
		} while (FindNextFile(hFound, &ffd));

		return files;
	}

	bool FilePath::Exists_Native(const wchar* path)
	{
		return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
	}

	bool FilePath::IsDirectory_Native(const wchar* path)
	{
		return GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY;
	}

	bool FilePath::IsDriveLetter_Native(const WString& drive)
	{
		if (drive.size() != 2)
			return false;

		return 
			((drive[0] >= L'A' && drive[0] <= L'Z')  ||
			 (drive[0] >= L'a' && drive[0] <= L'z')) &&
			drive[1] == L':';
	}
}
