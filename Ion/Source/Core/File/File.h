#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreUtilities.h"

namespace Ion
{
	namespace IO
	{
		enum FileType : byte
		{
			FT_Text   = 0,
			FT_Binary = 1,
			FT_Other  = 0xFF,
		};

		enum FileMode : byte
		{
			FM_Read      = Bitflag(0),
			FM_Write     = Bitflag(1),
			FM_Append    = Bitflag(2),
			FM_Reset     = Bitflag(3),
		};
	}

	/* File interface */
	class ION_API IFile
	{
	public:
		virtual bool Open(byte mode) = 0;
		virtual void Close() = 0;
		virtual bool Delete() = 0;

		virtual void Read(byte* outBuffer, ullong count) = 0;
		virtual void ReadLine(char* outBuffer, ullong count) = 0;
		virtual void ReadLine(std::string& outStr) = 0;
		virtual void Write(const byte* inBuffer, ullong count) = 0;
		virtual void WriteLine(const char* inBuffer, ullong count) = 0;
		virtual void WriteLine(const std::string& inStr) = 0;

		virtual bool IsOpen() const = 0;
		virtual bool Exists() const = 0;
	};

	/* Generic File abstract base class */
	class ION_API FileBase : public IFile
	{
	public:
		FileBase();
		FileBase(const std::wstring& filename);
		virtual ~FileBase() { }

		bool SetFilename(const std::wstring& filename);
	protected:
		/* If this function returns false, the filename will not be changed. */
		virtual bool SetFilename_Impl(const std::wstring& filename);
	public:
		FORCEINLINE std::wstring GetFilename() const { return m_Filename; }

		bool SetType(IO::FileType type);
	protected:
		/* If this function returns false, the type will not be changed. */
		virtual bool SetType_Impl(IO::FileType type);
	public: 
		FORCEINLINE IO::FileType GetType() const { return m_Type; }

	protected:
		std::wstring m_Filename;
		IO::FileType m_Type;
	};
}
