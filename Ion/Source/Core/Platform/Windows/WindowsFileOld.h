#pragma once

#include "Core/File/File.h"

namespace Ion
{
	class ION_API WindowsFileOld : public FileOld
	{
	public:
		WindowsFileOld();
		WindowsFileOld(const WString& filename);
		virtual ~WindowsFileOld() override;

		// File :
	protected:
		virtual bool SetFilename_Impl(const WString& filename) override;
		virtual bool SetType_Impl(IO::EFileType type) override;

		// End of File

		// IFile interface :
	public:
		virtual bool Open(uint8 mode) override;
		virtual void Close() override;
		virtual bool Delete() override;

		/* Reads data from a binary file and sets the file offset to the end of the read. */
		virtual bool Read(uint8* outBuffer, uint64 count) override;
		/* Reads data from a text file to string and sets the file offset to the end of the read. */
		virtual bool Read(String& outStr) override;
		/* Reads a line from a text file and sets the file offset to the beginning of the next line.
		   If the count parameter, hence the out buffer size, is not big enough it will only write 
		   to the end of the buffer and will move the offset to the end of the read section. */
		virtual bool ReadLine(char* outBuffer, uint64 count) override;
		/* Reads a line from a text file and sets the file offset to the beginning of the next line.
		   Will replace the specified string with the read content. */
		virtual bool ReadLine(String& outStr) override;
		/* Writes data to a binary file and sets the file offset to the end of the write. */
		virtual bool Write(const uint8* inBuffer, uint64 count) override;
		/* Writes string to a text file and sets the file offset to the end of the write. */
		virtual bool Write(const String& inStr) override;
		/* Writes a line to a text file and sets the file offset to the beginning of the next line. 
		   Parameter count is not a C string length, it is the buffer length (with the NULL character) */
		virtual bool WriteLine(const char* inBuffer, uint64 count) override;
		/* Writes a line to a text file and sets the file offset to the beginning of the next line. */
		virtual bool WriteLine(const String& inStr) override;

		virtual bool AddOffset(int64 count) override;
		virtual bool SetOffset(int64 count) override;
		virtual int64 GetOffset() const override;

		virtual int64 GetSize() const override;
		virtual WString GetExtension() const override;

		virtual bool IsDirectory() const override;
		virtual TArray<FileInfo> GetFilesInDirectory() const override;
		virtual WString FindInDirectoryRecursive(const WString& filename) const override;

		virtual bool IsOpen() const override;
		virtual bool Exists() const override;
		/* Returns true if the offset is greater than the filesize. */
		virtual bool EndOfFile() const override;

		// End of IFile interface

	private:
		HANDLE m_FileHandle;
		LARGE_INTEGER m_Offset;

		bool ReadLine_Internal(char* outBuffer, uint64 count, uint64* outReadCount, bool* bOutOverflow);
	};

	
}
