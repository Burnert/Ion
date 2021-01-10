#include "IonPCH.h"

#include "WindowsFile.h"
#include "Core/Platform/Windows/WindowsCore.h"

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
		// Handle errors first
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LOG_ERROR(TEXT("File {0} is already open!"), m_Filename);
			return false;
		}
		else if ((mode & (IO::FM_Read | IO::FM_Write)) == 0)
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
				LOG_WARN(TEXT("File {0} not found!"), m_Filename);

				m_FileHandle = CreateFile(m_Filename.c_str(), dwDesiredAccess, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
				LOG_TRACE(TEXT("File {0} created."), m_Filename);
			}
			else
			{
				WINDOWS_FORMAT_ERROR_MESSAGE(errorMsg, lastError);
				LOG_ERROR(TEXT("File {0} cannot be opened!\n{1}"), m_Filename, std::wstring(errorMsg));
				return false;
			}
		}

		LOG_TRACE(TEXT("File {0} opened."), m_Filename);
		return true;
	}

	void WindowsFile::Close()
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LOG_TRACE(TEXT("File {0} was closed."), m_Filename);
			CloseHandle(m_FileHandle);
			m_FileHandle = INVALID_HANDLE_VALUE;
		}
	}

	bool WindowsFile::Delete()
	{
		if (!DeleteFile(m_Filename.c_str()))
		{
			DWORD lastError = GetLastError();
			if (lastError == ERROR_FILE_NOT_FOUND)
			{
				LOG_ERROR(TEXT("File {0} cannot be deleted because it does not exist!"), m_Filename);
			}
			else if (lastError == ERROR_ACCESS_DENIED)
			{
				LOG_ERROR(TEXT("File {0} cannot be deleted. Access denied!"), m_Filename);
			}
			else
			{
				WINDOWS_FORMAT_ERROR_MESSAGE(errorMsg, lastError);
				LOG_ERROR(TEXT("File {0} cannot be deleted!\n{1}"), m_Filename, std::wstring(errorMsg));
			}
			return false;
		}
		LOG_INFO(TEXT("File {0} was deleted."), m_Filename);
		return true;
	}

	void WindowsFile::Read(byte* outBuffer, ullong count)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			if (m_Mode & IO::FM_Read)
			{
				LOG_DEBUG(TEXT("[Placeholder] Read File {0}"), m_Filename);
			}
		}
	}

	void WindowsFile::ReadLine(char* outBuffer, ullong size)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			if (m_Mode & IO::FM_Read)
			{
				LOG_DEBUG(TEXT("[Placeholder] ReadLine File {0}"), m_Filename);
			}
		}
	}

	void WindowsFile::ReadLine(std::string& outStr)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			if (m_Mode & IO::FM_Read)
			{
				LOG_DEBUG(TEXT("[Placeholder] ReadLine File {0}"), m_Filename);
			}
		}
	}

	void WindowsFile::Write(const byte* inBuffer, ullong count)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			if (m_Mode & IO::FM_Write)
			{
				ulong bytesWritten;
				if (!WriteFile(m_FileHandle, inBuffer, (DWORD)count, &bytesWritten, NULL))
				{
					Windows::PrintLastError(TEXT("Cannot write file! {0}"), m_Filename);
				}
				LOG_DEBUG("Written {0} bytes.", bytesWritten);
			}
		}
	}

	void WindowsFile::WriteLine(const char* inBuffer, ullong count)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			if (m_Mode & IO::FM_Write)
			{
				// Copy the inBuffer and set the last character to NewLine
				char* tempBuffer = new char[count + 1];
				memcpy_s(tempBuffer, count, inBuffer, count);
				tempBuffer[count] = '\n';

				ulong bytesWritten;
				if (!WriteFile(m_FileHandle, tempBuffer, (DWORD)(count + 1), &bytesWritten, NULL))
				{
					Windows::PrintLastError(TEXT("Cannot write file! {0}"), m_Filename);
				}
				LOG_DEBUG("Written {0} bytes.", bytesWritten);
				delete[] tempBuffer;
			}
		}
	}

	void WindowsFile::WriteLine(const std::string& inStr)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			if (m_Mode & IO::FM_Write)
			{
				WriteLine(inStr.c_str(), inStr.size());
			}
		}
	}

	bool WindowsFile::IsOpen() const
	{
		return m_FileHandle != INVALID_HANDLE_VALUE;
	}

	bool WindowsFile::Exists() const
	{
		DWORD attributes = GetFileAttributes(m_Filename.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES && 
			!(attributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool WindowsFile::AddOffset(llong count)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER _count;
			_count.QuadPart = count;
			if (!SetFilePointerEx(m_FileHandle, _count, &m_Offset, FILE_CURRENT))
			{
				Windows::PrintLastError(TEXT("Cannot add file offset! {0}"), m_Filename);
				return false;
			}
			return true;
		}
		return false;
	}

	bool WindowsFile::SetOffset(llong count)
	{
		if (m_FileHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER _count;
			_count.QuadPart = count;
			if (!SetFilePointerEx(m_FileHandle, _count, &m_Offset, FILE_BEGIN))
			{
				Windows::PrintLastError(TEXT("Cannot set file offset! {0}"), m_Filename);
				return false;
			}
			return true;
		}
		return false;
	}
}
