#pragma once

#include "Core/File/File.h"

namespace Ion
{
	class ION_API WindowsFile : public File
	{
	public:
		WindowsFile();
		WindowsFile(const std::wstring& filename);
		virtual ~WindowsFile() override;

		// File :
	protected:
		virtual bool SetFilename_Impl(const std::wstring& filename) override;
		virtual bool SetType_Impl(IO::EFileType type) override;

		// End of File

		// IFile interface :
	public:
		virtual bool Open(ubyte mode) override;
		virtual void Close() override;
		virtual bool Delete() override;

		/* Reads data from binary file and sets the file offset to the end of the read. */
		virtual bool Read(ubyte* outBuffer, ullong count) override;
		/* Reads a line from a text file and sets the file offset to the beginning of the next line.
		   If the count parameter, hence the out buffer size, is not big enough it will only write 
		   to the end of the buffer and will move the offset to the end of the read section. */
		virtual bool ReadLine(char* outBuffer, ullong count) override;
		/* Reads a line from a text file and sets the file offset to the beginning of the next line.
		   Will replace the specified string with the read content. */
		virtual bool ReadLine(std::string& outStr) override;
		/* Writes data to binary file and sets the file offset to the end of the write. */
		virtual bool Write(const ubyte* inBuffer, ullong count) override;
		/* Writes a line to a text file and sets the file offset to the beginning of the next line. 
		   Parameter count is not a C string length, it is the buffer length (with the NULL character) */
		virtual bool WriteLine(const char* inBuffer, ullong count) override;
		/* Writes a line to a text file and sets the file offset to the beginning of the next line. */
		virtual bool WriteLine(const std::string& inStr) override;

		virtual bool AddOffset(llong count) override;
		virtual bool SetOffset(llong count) override;
		virtual llong GetOffset() const override;

		virtual llong GetSize() const override;
		virtual WString GetExtension() const override;

		virtual bool IsOpen() const override;
		virtual bool Exists() const override;
		/* Returns true if the offset is greater than the filesize. */
		virtual bool EndOfFile() const override;

		// End of IFile interface

	private:
		HANDLE m_FileHandle;
		LARGE_INTEGER m_Offset;

		bool ReadLine_Internal(char* outBuffer, ullong count, ullong* outReadCount, bool* bOutOverflow);
	};
}
