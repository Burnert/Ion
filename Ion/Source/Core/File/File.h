#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreUtility.h"

namespace Ion
{
	namespace IO
	{
		enum EFileType : ubyte
		{
			FT_Text   = 1,
			FT_Binary = 2,
			FT_Other  = 0xFF,
		};

		enum EFileMode : ubyte
		{
			FM_Read      = Bitflag(0),
			FM_Write     = Bitflag(1),
			FM_Append    = Bitflag(2),
			FM_Reset     = Bitflag(3),
		};

		enum ENewLineType : ubyte
		{
			NLT_LF      = 1,
			NLT_CR      = 2,
			NLT_CRLF    = 3,
		};
	}

	/* File interface */
	class ION_API IFile
	{
		// @TODO: Add an option to create directories (also recursively)
	public:
		virtual bool Open(ubyte mode) = 0;
		virtual void Close() = 0;
		virtual bool Delete() = 0;

		virtual bool Read(ubyte* outBuffer, ullong count) = 0;
		virtual bool ReadLine(char* outBuffer, ullong count) = 0;
		virtual bool ReadLine(std::string& outStr) = 0;
		virtual bool Write(const ubyte* inBuffer, ullong count) = 0;
		virtual bool WriteLine(const char* inBuffer, ullong count) = 0;
		virtual bool WriteLine(const std::string& inStr) = 0;

		virtual bool AddOffset(llong count) = 0;
		virtual bool SetOffset(llong count) = 0;
		virtual llong GetOffset() const = 0;

		virtual llong GetSize() const = 0;
		virtual WString GetExtension() const = 0;

		virtual bool IsOpen() const = 0;
		virtual bool Exists() const = 0;
		virtual bool EndOfFile() const = 0;

		inline bool Read(char* outBuffer, ullong count)
		{
			return Read((ubyte*)outBuffer, count);
		}
		template<ullong Size>
		inline bool Read(char(&outBuffer)[Size])
		{
			return Read((ubyte*)outBuffer, Size - 1);
		}

		inline bool Write(const char* inBuffer, ullong count)
		{
			return Write((const ubyte*)inBuffer, count);
		}
		template<ullong Size>
		inline bool Write(const char(&inBuffer)[Size])
		{
			return Write((const ubyte*)inBuffer, Size - 1);
		}
	};

	/* Generic File abstract base class */
	class ION_API File : public IFile
	{
	public:
		static File* Create();
		static File* Create(const std::wstring& filename);

		File();
		File(const std::wstring& filename);
		virtual ~File() { }
		
		// @TODO: Add copy and move constructors

		/* This is the type of the new line character that will be written in a text file. */
		IO::ENewLineType WriteNewLineType;

		bool SetFilename(const std::wstring& filename);
	protected:
		/* If this function returns false, the filename will not be changed. */
		virtual bool SetFilename_Impl(const std::wstring& filename);
	public:
		FORCEINLINE std::wstring GetFilename() const { return m_Filename; }

		bool SetType(IO::EFileType type);
	protected:
		/* If this function returns false, the type will not be changed. */
		virtual bool SetType_Impl(IO::EFileType type);
	public: 
		FORCEINLINE IO::EFileType GetType() const { return m_Type; }

		FORCEINLINE bool IsFilenameValid() const { return m_Filename != TEXT(""); }

	protected:
		std::wstring m_Filename;
		IO::EFileType m_Type;
		ubyte m_Mode;

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

	using Directory = File;
}
