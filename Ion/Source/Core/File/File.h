#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreUtility.h"

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

		enum NewLineType : byte
		{
			NLT_LF      = 1,
			NLT_CR      = 2,
			NLT_CRLF    = 3,
		};
	}

	/* File interface */
	class ION_API IFile
	{
	public:
		virtual bool Open(byte mode) = 0;
		virtual void Close() = 0;
		virtual bool Delete() = 0;

		virtual bool Read(byte* outBuffer, ullong count) = 0;
		virtual bool ReadLine(char* outBuffer, ullong count) = 0;
		virtual bool ReadLine(std::string& outStr) = 0;
		virtual bool Write(const byte* inBuffer, ullong count) = 0;
		virtual bool WriteLine(const char* inBuffer, ullong count) = 0;
		virtual bool WriteLine(const std::string& inStr) = 0;

		virtual bool AddOffset(llong count) = 0;
		virtual bool SetOffset(llong count) = 0;
		virtual llong GetOffset() const = 0;

		virtual llong GetSize() const = 0;

		virtual bool IsOpen() const = 0;
		virtual bool Exists() const = 0;
		virtual bool EndOfFile() const = 0;
	};

	/* Generic File abstract base class */
	class ION_API FileBase : public IFile
	{
	public:
		FileBase();
		FileBase(const std::wstring& filename);
		virtual ~FileBase() { }

		/* This is the type of the new line character that will be written in a text file. */
		IO::NewLineType WriteNewLineType;

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

		FORCEINLINE bool IsFilenameValid() const { return m_Filename != TEXT(""); }

	protected:
		std::wstring m_Filename;
		IO::FileType m_Type;
		byte m_Mode;

		// Caches ----------------------

		mutable llong m_FileSize;
		/* Invalidates file size cache and retrieves the new size. */
		FORCEINLINE void UpdateFileSizeCache() const { m_FileSize = -1; GetSize(); }
		/* Sets the file size cache to the size specified. */
		FORCEINLINE void UpdateFileSizeCache(llong newFileSize) const { m_FileSize = newFileSize; }

		// Debug only code -------------
#ifdef ION_DEBUG
		bool m_DebugLog = false;
	public:
		/* Do not call this in any other configuration other than Debug!
		   You can wrap this call in a DEBUG() macro for automation.
		   Enables low level logging for operations on this file. */
		FORCEINLINE void EnableDebugLog() { m_DebugLog = true; }
#endif
	};
}