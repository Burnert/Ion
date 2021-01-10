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

		virtual bool Read(byte* outBuffer, ullong count) override;
		virtual bool ReadLine(char* outBuffer, ullong count) override;
		virtual bool ReadLine(std::string& outStr) override;
		virtual bool Write(const byte* inBuffer, ullong count) override;
		virtual bool WriteLine(const char* inBuffer, ullong count) override;
		virtual bool WriteLine(const std::string& inStr) override;

		virtual bool AddOffset(llong count) override;
		virtual bool SetOffset(llong count) override;

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
	};
	
	using File = WindowsFile;
}
