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

		virtual void Read(byte* outBuffer, ullong count) override;
		virtual void ReadLine(char* outBuffer, ullong count) override;
		virtual void ReadLine(std::string& outStr) override;
		virtual void Write(const byte* inBuffer, ullong count) override;
		virtual void WriteLine(const char* inBuffer, ullong count) override;
		virtual void WriteLine(const std::string& inStr) override;

		virtual bool IsOpen() const override;
		virtual bool Exists() const override;

		// End of IFile interface

		bool AddOffset(llong count);
		bool SetOffset(llong count);

	private:
		HANDLE m_FileHandle;
		LARGE_INTEGER m_Offset;
		byte m_Mode;
	};
	
	using File = WindowsFile;
}
