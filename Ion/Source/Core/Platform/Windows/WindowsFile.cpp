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
		: WindowsFile(L"")
	{ }

	WindowsFile::WindowsFile(const std::wstring& filename) :
		FileBase(filename),
		m_FileHandle(INVALID_HANDLE_VALUE),
		m_Offset({ 0 }),
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
		if ((mode & (IO::FM_Read | IO::FM_Write)) == 0)
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
			if (lastError == ERROR_FILE_NOT_FOUND)
			{
				LOG_WARN(TEXT("File '{0}' not found!"), m_Filename);

				m_FileHandle = CreateFile(m_Filename.c_str(), dwDesiredAccess, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
				LOG_TRACE(TEXT("File '{0}' created."), m_Filename);
			}
			else
			{
				WINDOWS_FORMAT_ERROR_MESSAGE(errorMsg, lastError);
				LOG_ERROR(TEXT("File '{0}' cannot be opened!\n{1}"), m_Filename, std::wstring(errorMsg));
				return false;
			}
		}

		LOG_TRACE(TEXT("File '{0}' opened."), m_Filename);
		return true;
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
			Windows::PrintLastError(TEXT("Cannot write file! '{0}'"), m_Filename);
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
			Windows::PrintLastError(TEXT("Cannot read file! '{0}'"), m_Filename);
			return false;
		}
		LOG_DEBUG("Read {0} bytes.", bytesRead);
		return true;
	}

	bool WindowsFile::ReadLine(char* outBuffer, ullong size)
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

		LOG_DEBUG(TEXT("[Placeholder] ReadLine File '{0}'"), m_Filename);
		return true;
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

		LOG_DEBUG(TEXT("[Placeholder] ReadLine File '{0}'"), m_Filename);
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
			Windows::PrintLastError(TEXT("Cannot write file! '{0}'"), m_Filename);
			return false;
		}
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
			Windows::PrintLastError(TEXT("Cannot write file! '{0}'"), m_Filename);
			return false;
		}
		delete[] tempBuffer;

		LOG_DEBUG("Written {0} bytes.", bytesWritten);
		return true;
	}

	bool WindowsFile::WriteLine(const std::string& inStr)
	{
		return WriteLine(inStr.c_str(), inStr.size());
	}

	bool WindowsFile::Exists() const
	{
		DWORD attributes = GetFileAttributes(m_Filename.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES &&
			!(attributes & FILE_ATTRIBUTE_DIRECTORY);
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
			Windows::PrintLastError(TEXT("Cannot add file offset! '{0}'"), m_Filename);
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
			Windows::PrintLastError(TEXT("Cannot set file offset! '{0}'"), m_Filename);
			return false;
		}
		return true;
	}
}
