#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreUtility.h"

namespace Ion
{
	namespace IO
	{
		enum EFileType : uint8
		{
			FT_Text   = 1,
			FT_Binary = 2,
			FT_Other  = 0xFF,
		};

		enum EFileMode : uint8
		{
			FM_Read      = Bitflag(0),
			FM_Write     = Bitflag(1),
			FM_Append    = Bitflag(2),
			FM_Reset     = Bitflag(3),
		};

		enum ENewLineType : uint8
		{
			NLT_LF      = 1,
			NLT_CR      = 2,
			NLT_CRLF    = 3,
		};
	}

	struct FileInfo
	{
		WString Filename;
		WString FullPath;
		int64 Size;
		bool bDirectory;
	};

	/* File interface */
	class ION_API IFile
	{
		// @TODO: Add an option to create directories (also recursively)
	public:
		virtual bool Open(uint8 mode) = 0;
		virtual void Close() = 0;
		virtual bool Delete() = 0;

		virtual bool Read(uint8* outBuffer, uint64 count) = 0;
		virtual bool Read(String& outStr) = 0;
		virtual bool ReadLine(char* outBuffer, uint64 count) = 0;
		virtual bool ReadLine(String& outStr) = 0;
		virtual bool Write(const uint8* inBuffer, uint64 count) = 0;
		virtual bool Write(const String& inStr) = 0;
		virtual bool WriteLine(const char* inBuffer, uint64 count) = 0;
		virtual bool WriteLine(const String& inStr) = 0;

		virtual bool AddOffset(int64 count) = 0;
		virtual bool SetOffset(int64 count) = 0;
		virtual int64 GetOffset() const = 0;

		virtual int64 GetSize() const = 0;
		virtual WString GetExtension() const = 0;

		virtual bool IsDirectory() const = 0;
		virtual TArray<FileInfo> GetFilesInDirectory() const = 0;
		virtual WString FindInDirectoryRecursive(const WString& filename) const = 0;

		virtual bool IsOpen() const = 0;
		virtual bool Exists() const = 0;
		virtual bool EndOfFile() const = 0;
	};

	/* Generic File abstract base class */
	class ION_API File : public IFile
	{
	public:
		static File* Create();
		static File* Create(const WString& filename);

		File();
		File(const WString& filename);
		virtual ~File() { }
		
		// @TODO: Add copy and move constructors

		// IFile:

		virtual bool Read(uint8* outBuffer, uint64 count) = 0;
		virtual bool Read(String& outStr) = 0;
		virtual bool Write(const uint8* inBuffer, uint64 count) = 0;
		virtual bool Write(const String& inStr) = 0;

		// End of IFile:

		// IFile wrappers:

		inline bool Read(char* outBuffer, uint64 count)
		{
			return Read((uint8*)outBuffer, count);
		}
		template<uint64 Size>
		inline bool Read(char(&outBuffer)[Size])
		{
			return Read((uint8*)outBuffer, Size - 1);
		}

		inline bool Write(const char* inBuffer, uint64 count)
		{
			return Write((const uint8*)inBuffer, count);
		}
		template<uint64 Size>
		inline bool Write(const char(&inBuffer)[Size])
		{
			return Write((const uint8*)inBuffer, Size - 1);
		}

		// End of IFile wrappers

		/* This is the type of the new line character that will be written in a text file. */
		IO::ENewLineType WriteNewLineType;

		bool SetFilename(const WString& filename);
	protected:
		/* If this function returns false, the filename will not be changed. */
		virtual bool SetFilename_Impl(const WString& filename);
	public:
		FORCEINLINE WString GetFilename() const { return m_Filename; }

		bool SetType(IO::EFileType type);
	protected:
		/* If this function returns false, the type will not be changed. */
		virtual bool SetType_Impl(IO::EFileType type);
	public: 
		FORCEINLINE IO::EFileType GetType() const { return m_Type; }

		FORCEINLINE bool IsFilenameValid() const { return m_Filename != TEXT(""); }

	protected:
		WString m_Filename;
		IO::EFileType m_Type;
		uint8 m_Mode;

		// Caches ----------------------

		mutable int64 m_FileSize;
		/* Invalidates file size cache and retrieves the new size. */
		FORCEINLINE void UpdateFileSizeCache() const { m_FileSize = -1; GetSize(); }
		/* Sets the file size cache to the size specified. */
		FORCEINLINE void UpdateFileSizeCache(int64 newFileSize) const { m_FileSize = newFileSize; }

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
