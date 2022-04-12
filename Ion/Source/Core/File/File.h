#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreUtility.h"

#pragma warning(disable:26812)

#if ION_DEBUG
#define ENABLE_LOG(file) file.EnableDebugLog()
#else
#define ENABLE_LOG(file)
#endif

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

		FileInfo(const WString& filename, const WString& fullPath, int64 size, bool bDir) :
			Filename(filename),
			FullPath(fullPath),
			Size(size),
			bDirectory(bDir)
		{ }
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
	class ION_API FileOld : public IFile
	{
	public:
		static FileOld* Create();
		static FileOld* Create(const WString& filename);

		FileOld();
		FileOld(const WString& filename);
		virtual ~FileOld() { }
		
		// @TODO: Add copy and move constructors

		static bool LoadToString(const WString& filename, String& outStr);

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

	//-----------------------------------------------
	// New File API ---------------------------------
	//-----------------------------------------------

	namespace EFileMode
	{
		enum Type : uint8
		{
			Read      = Bitflag(0),
			Write     = Bitflag(1),
			Append    = Bitflag(2),
			Reset     = Bitflag(3),
			CreateNew = Bitflag(4),
			DoNotOpen = Bitflag(5),
		};
	}

	enum class EFileType : uint8
	{
		Text   = 1,
		Binary = 2,
		Other  = 0xFF,
	};

	enum class ENewLineType : uint8
	{
		LF   = 1,
		CR   = 2,
		CRLF = 3,
	};

	enum class EFilePathValidation : uint8
	{
		Unchecked = 0,
		Checked   = 1,
	};

	struct FileList
	{
		using FileInfoArray = TArray<FileInfo>;

		using Iterator             = FileInfoArray::iterator;
		using ConstIterator        = FileInfoArray::const_iterator;
		using ReverseIterator      = FileInfoArray::reverse_iterator;
		using ConstReverseIterator = FileInfoArray::const_reverse_iterator;

		FileInfoArray Files;

		uint32 GetCount() const { return (uint32)Files.size(); }
		bool IsEmpty() const { return Files.empty(); }

		FileList(FileInfoArray&& files) noexcept
			: Files(files)
		{ }

		FileList(const FileList&) = default;
		FileList(FileList&&) noexcept = default;

		FileList& operator=(const FileList&) = default;
		FileList& operator=(FileList&&) noexcept = default;

		inline Iterator begin() { return Files.begin(); }
		inline Iterator end()   { return Files.end();   }

		inline ConstIterator begin() const { return Files.begin(); }
		inline ConstIterator end() const   { return Files.end();   }

		inline ReverseIterator rbegin() { return Files.rbegin(); }
		inline ReverseIterator rend()   { return Files.rend();   }

		inline ConstReverseIterator rbegin() const { return Files.rbegin(); }
		inline ConstReverseIterator rend() const   { return Files.rend();   }
	};

#if ION_PLATFORM_WINDOWS
	struct WindowsFileData
	{
		friend class File;
	private:
		WindowsFileData() :
			m_FileHandle((void*)(int64)-1 /* INVALID_HANDLE_VALUE */)
		{ }
		
		void* m_FileHandle;
	};

	using NativeFile = WindowsFileData;
#endif

	class FilePath;

	class ION_API File
	{
		friend class FilePath;
	public:
		File(const FilePath& path, uint8 mode = EFileMode::DoNotOpen);
		File(const WString& filename, uint8 mode = EFileMode::DoNotOpen);

		bool Open(uint8 mode);

		static bool Delete(const FilePath& path);
		static inline bool Delete(const WString& filename)
		{
			return Delete_Native(filename);
		}

		void Close();

		// Read functions

		inline bool Read(uint8* outBuffer, uint64 count)
		{
			ionassert(m_Mode & IO::FM_Read, "Read access mode was not specified when opening the file.");

			return Read_Native(outBuffer, count);
		}

		inline bool Read(char* outBuffer, uint64 count)
		{
			return Read((uint8*)outBuffer, count);
		}

		bool Read(String& outStr);

		inline bool ReadLine(char* outBuffer, uint64 count)
		{
			ionassert(m_Mode & IO::FM_Read, "Read access mode was not specified when opening the file.");

			return ReadLine_Native(outBuffer, count);
		}

		inline bool ReadLine(String& outStr)
		{
			ionassert(m_Mode & IO::FM_Read, "Read access mode was not specified when opening the file.");

			return ReadLine_Native(outStr);
		}

		template<uint64 Size>
		inline bool Read(uint8(&outBuffer)[Size])
		{
			return Read((uint8*)outBuffer, Size);
		}

		template<uint64 Size>
		inline bool Read(char(&outBuffer)[Size])
		{
			return Read((uint8*)outBuffer, Size - 1);
		}

		static bool ReadToString(const FilePath& filePath, String& outString);
		static bool ReadToString(const WString& filePath, String& outString);

		// Write functions

		inline bool Write(const uint8* inBuffer, uint64 count)
		{
			ionassert(m_Mode & IO::FM_Write, "Write access mode was not specified when opening the file.");

			return Write_Native(inBuffer, count);
		}

		inline bool Write(const char* inBuffer, uint64 count)
		{
			return Write((const uint8*)inBuffer, count);
		}

		inline bool Write(const String& inStr)
		{
			return Write(inStr.c_str(), inStr.size());
		}

		inline bool WriteLine(const char* inBuffer, uint64 count, ENewLineType newLineType = s_DefaultNewLineType)
		{
			ionassert(m_Mode & IO::FM_Write, "Write access mode was not specified when opening the file.");

			return WriteLine_Native(inBuffer, count, newLineType);
		}

		inline bool WriteLine(const String& inStr, ENewLineType newLineType = s_DefaultNewLineType)
		{
			return WriteLine(inStr.c_str(), inStr.size() + 1, newLineType);
		}

		template<uint64 Size>
		inline bool Write(const uint8(&inBuffer)[Size])
		{
			return Write((const uint8*)inBuffer, Size);
		}

		template<uint64 Size>
		inline bool Write(const char(&inBuffer)[Size])
		{
			return Write((const uint8*)inBuffer, Size - 1);
		}

		// Other functions

		inline bool AddOffset(int64 count)
		{
			return AddOffset_Native(count);
		}

		inline bool SetOffset(int64 count)
		{
			return SetOffset_Native(count);
		}

		inline int64 GetOffset() const
		{
			return m_Offset;
		}

		int64 GetSize() const;

		inline const WString& GetExtension() const
		{
			return m_FileExtension;
		}

		inline bool IsOpen() const
		{
			return m_bOpen;
		}

		bool Exists() const;

		bool EndOfFile() const;

		bool IsDirectory() const;

		FilePath GetFilePath(EFilePathValidation validation = EFilePathValidation::Unchecked) const;

		inline const WString& GetFullPath() const
		{
			return m_FilePath;
		}

		static bool IsFileNameLegal(const WString& name);

		~File();

		File(const File&) = delete;
		File(File&&) noexcept = default;

		File& operator=(const File&) = delete;
		File& operator=(File&&) = default;

	private:
		WString m_FilePath;
		EFileType m_Type;
		uint8 m_Mode;
		bool m_bOpen;

		int64 m_Offset; 

	// Caches

		mutable int64 m_FileSize;
		/* Invalidates file size cache and retrieves the new size. */
		FORCEINLINE void UpdateFileSizeCache() const { m_FileSize = -1; GetSize(); }
		/* Sets the file size cache to the size specified. */
		FORCEINLINE void UpdateFileSizeCache(int64 newFileSize) const { m_FileSize = newFileSize; }

		mutable WString m_FileExtension;
		void UpdateFileExtensionCache() const;

	// End of Caches

	// Platform specific

	private:
		bool Open_Native();
		static bool Delete_Native(const WString& filename);
		bool Close_Native();

		bool Read_Native(uint8* outBuffer, uint64 count);
		bool ReadLine_Internal(char* outBuffer, uint64 count, uint64* outReadCount, bool* bOutOverflow);
		bool ReadLine_Native(char* outBuffer, uint64 count);
		bool ReadLine_Native(String& outStr);
		bool Write_Native(const uint8* inBuffer, uint64 count);
		bool WriteLine_Native(const char* inBuffer, uint64 count, ENewLineType newLineType);

		bool AddOffset_Native(int64 count);
		bool SetOffset_Native(int64 count);

		NativeFile m_NativeFile;

		static const ENewLineType s_DefaultNewLineType;

#if ION_PLATFORM_WINDOWS
		static constexpr wchar File::s_IllegalCharacters[] = { L'/', L'\\', L'*', L'<', L'>', L'|', L'?', L':', L'"' };
#else
		static constexpr wchar File::s_IllegalCharacters[] = { L'\0' };
#endif

	// End of Platform specific

#if ION_DEBUG
	private:
		bool m_DebugLog = false;
		static bool s_GlobalDebugLog;
	public:
		/* Do not call this in any other configuration other than Debug!
		   You can wrap this call in a DEBUG() macro for automation.
		   Enables low level logging for operations on this file. */
		FORCEINLINE void EnableDebugLog() { m_DebugLog = true; }
		FORCEINLINE void EnableGlobalDebugLog() { s_GlobalDebugLog = true; }
#endif
	};

#if ION_PLATFORM_WINDOWS

#endif

	class ION_API FilePath
	{
		friend class File;
	public:
		static constexpr const uint64 MaxPathLength = 2000;

		FilePath();
		FilePath(const WString& path, EFilePathValidation validation = EFilePathValidation::Unchecked);
		FilePath(const FilePath& path, EFilePathValidation validation);

		FilePath(const FilePath&) = default;
		FilePath(FilePath&&) noexcept = default;

		void Set(const WString& path);
		void Set(const FilePath& path);

		bool ChangeDirectory(const WString& directory);
		bool ChangePath(const FilePath& path);

		inline void Back()
		{
			m_Path.pop_back();
			UpdatePathName();
		}

		inline bool Make(const WString& name)
		{
			ionassert(File::IsFileNameLegal(name));

			return Make_Native(name.c_str());
		}

		inline bool Rename(const WString& newName)
		{
			ionassert(File::IsFileNameLegal(newName));

			// @TODO: Implement this
			return true;
		}

		bool Delete(bool bForce = false);

		inline bool Exists() const
		{
			return Exists(m_PathName.c_str());
		}

		inline bool IsDirectory() const
		{
			return IsDirectory(m_PathName.c_str());
		}

		inline bool IsFile() const
		{
			return Exists() && !IsDirectory();
		}

		FileList ListFiles() const;
		//WString Find(const wchar* name) const;
		//WString FindRecursive(const wchar* name) const;

		bool IsRelative() const;

		inline void SetValidation(EFilePathValidation validation)
		{
			m_bChecked = (bool)validation;
		}

		inline const WString& ToString() const
		{
			return m_PathName;
		}

		inline static bool Exists(const wchar* path)
		{
			return Exists_Native(path);
		}

		inline static bool Exists(const WString& path)
		{
			return Exists(path.c_str());
		}

		inline static bool IsDirectory(const wchar* path)
		{
			return IsDirectory_Native(path);
		}

		inline static bool IsDirectory(const WString& path)
		{
			return IsDirectory(path.c_str());
		}

		inline static bool IsFile(const wchar* path)
		{
			return Exists(path) && !IsDirectory(path);
		}

		inline static bool IsFile(const WString& path)
		{
			return IsFile(path.c_str());
		}

		static FileList ListFiles(const wchar* path);
		//static WString Find(const wchar* path, const wchar* name);
		//static WString FindRecursive(const wchar* path, const wchar* name);

		FilePath& operator=(const FilePath&) = default;
		FilePath& operator=(FilePath&&) = default;
		FilePath& operator=(const WString&);
		FilePath& operator=(WString&&);

		FilePath& operator+=(const FilePath& path);
		FilePath& operator+=(const WString& directory);

		FilePath operator+(const FilePath& path) const;
		FilePath operator+(const WString& directory) const;

		bool operator==(const FilePath& path) const;
		bool operator==(const WString& path) const;

		bool operator!=(const FilePath& path) const;
		bool operator!=(const WString& path) const;

		inline operator WString() const
		{
			return ToString();
		}

	private:
		static TArray<WString> SplitPathName(const WString& path);
		static WStringView StripSlashes(const WString& name);

		// Platform specific

		bool Make_Native(const wchar* name);
		bool Delete_Native();
		bool DeleteForce_Native();
		static bool Exists_Native(const wchar* path);
		static bool IsDirectory_Native(const wchar* path);
		TArray<FileInfo> ListFiles_Native() const;

	private:
		TArray<WString> m_Path;
		bool m_bChecked;
		
		mutable WString m_PathName;
		void UpdatePathName() const;
	};

	inline FilePath& FilePath::operator=(const WString& str)
	{
		Set(str);
	}

	inline FilePath& FilePath::operator=(WString&& str)
	{
		Set(str);
	}
}
