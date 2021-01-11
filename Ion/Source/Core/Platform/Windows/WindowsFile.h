#pragma once

#include "Core/File/File.h"

namespace Ion
{
	class ION_API WindowsFile : public FileBase
	{
	public:
		WindowsFile();
		WindowsFile(const std::wstring& filename);
		virtual ~WindowsFile() override;

		// FileBase :
	protected:
		virtual bool SetFilename_Impl(const std::wstring& filename) override;
		virtual bool SetType_Impl(IO::FileType type) override;

		// End of FileBase

		// IFile interface :
	public:
		virtual bool Open(byte mode) override;
		virtual void Close() override;
		virtual bool Delete() override;

		// @TODO: Make this work with CRLF line endings (now it only works with LF)

		/* Reads data from binary file and sets the file offset to the end of the read. */
		virtual bool Read(byte* outBuffer, ullong count) override;
		/* Reads a line from a text file and sets the file offset to the beginning of the next line.
		   If the count parameter, hence the out buffer size, is not big enough it will only write 
		   to the end of the buffer and will move the offset to the end of the read section. */
		virtual bool ReadLine(char* outBuffer, ullong count) override;
		/* Reads a line from a text file and sets the file offset to the beginning of the next line.
		   Will replace the specified string with the read content. */
		virtual bool ReadLine(std::string& outStr) override;
		/* Writes data to binary file and sets the file offset to the end of the write. */
		virtual bool Write(const byte* inBuffer, ullong count) override;
		/* Writes a line to a text file and sets the file offset to the beginning of the next line. 
		   Parameter count is not a C string length, it is the buffer length (with the NULL character) */
		virtual bool WriteLine(const char* inBuffer, ullong count) override;
		/* Writes a line to a text file and sets the file offset to the beginning of the next line. */
		virtual bool WriteLine(const std::string& inStr) override;

		virtual bool AddOffset(llong count) override;
		virtual bool SetOffset(llong count) override;

		virtual llong GetSize() const override;

		FORCEINLINE virtual bool IsOpen() const override
		{
			return m_FileHandle != INVALID_HANDLE_VALUE;
		}
		virtual bool Exists() const override;

		// End of IFile interface

	private:
		HANDLE m_FileHandle;
		LARGE_INTEGER m_Offset;
		byte m_Mode;

		bool ReadLine_Internal(char* outBuffer, ullong count, ullong* outReadCount, bool* bOutOverflow);
	};
	
	using File = WindowsFile;
}
