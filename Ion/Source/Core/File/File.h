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
#pragma region Enums

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

#pragma endregion

#pragma region FileList

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

	struct FileList
	{
		using FileInfoArray = TArray<FileInfo>;

		using Iterator             = FileInfoArray::iterator;
		using ConstIterator        = FileInfoArray::const_iterator;
		using ReverseIterator      = FileInfoArray::reverse_iterator;
		using ConstReverseIterator = FileInfoArray::const_reverse_iterator;

		FileInfoArray Files;

		template<typename Pred>
		FileList Filter(Pred pred) const;

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

	template<typename Pred>
	inline FileList FileList::Filter(Pred pred) const
	{
		FileInfoArray filtered;
		filtered.reserve(Files.size());
		for (const FileInfo& info : Files)
		{
			if (pred(info))
			{
				filtered.push_back(info);
			}
		}
		filtered.shrink_to_fit();
		return FileList(Move(filtered));
	}

#pragma endregion

// File class -------------------------------------------------------------------------------

#pragma region File

	class FilePath;

	class ION_API File
	{
	public:
		File(const FilePath& path, uint8 mode = EFileMode::DoNotOpen);
		File(const WString& filename, uint8 mode = EFileMode::DoNotOpen);

		bool Open(uint8 mode);

		static bool Delete(const FilePath& path);
		static bool Delete(const WString& filename);

		void Close();

		// Read functions

		bool Read(uint8* outBuffer, uint64 count);

		bool Read(char* outBuffer, uint64 count);

		bool Read(String& outStr);

		bool ReadLine(char* outBuffer, uint64 count);

		bool ReadLine(String& outStr);

		template<uint64 Size>
		bool Read(uint8(&outBuffer)[Size]);

		template<uint64 Size>
		bool Read(char(&outBuffer)[Size]);

		static bool ReadToString(const FilePath& filePath, String& outString);
		static bool ReadToString(const WString& filePath, String& outString);

		// Write functions

		bool Write(const uint8* inBuffer, uint64 count);

		bool Write(const char* inBuffer, uint64 count);

		bool Write(const String& inStr);

		bool WriteLine(const char* inBuffer, uint64 count, ENewLineType newLineType = s_DefaultNewLineType);

		bool WriteLine(const String& inStr, ENewLineType newLineType = s_DefaultNewLineType);

		template<uint64 Size>
		bool Write(const uint8(&inBuffer)[Size]);

		template<uint64 Size>
		bool Write(const char(&inBuffer)[Size]);

		// Other functions

		bool AddOffset(int64 count);

		bool SetOffset(int64 count);

		int64 GetOffset() const;

		int64 GetSize() const;

		const WString& GetExtension() const;

		bool IsOpen() const;

		bool Exists() const;

		bool EndOfFile() const;

		bool IsDirectory() const;

		FilePath GetFilePath(EFilePathValidation validation = EFilePathValidation::Unchecked) const;

		const WString& GetFullPath() const;

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

		void SetNativePointer_Native();

		void* m_NativePointer;

		static const ENewLineType s_DefaultNewLineType;

#if ION_PLATFORM_WINDOWS
		static constexpr wchar File::s_IllegalCharacters[] = { L'/', L'\\', L'*', L'<', L'>', L'|', L'?', L':', L'"' };
#else
		static constexpr wchar File::s_IllegalCharacters[] = { L'\0' };
#endif
		friend void*& GetNative(File* file);
		friend void* const& GetNative(const File* file);

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
		friend class FilePath;
	};

	inline bool File::Delete(const WString& filename)
	{
		return Delete_Native(filename);
	}

	inline bool File::Read(uint8* outBuffer, uint64 count)
	{
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		return Read_Native(outBuffer, count);
	}

	inline bool File::Read(char* outBuffer, uint64 count)
	{
		return Read((uint8*)outBuffer, count);
	}

	inline bool File::ReadLine(char* outBuffer, uint64 count)
	{
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		return ReadLine_Native(outBuffer, count);
	}

	inline bool File::ReadLine(String& outStr)
	{
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		return ReadLine_Native(outStr);
	}

	template<uint64 Size>
	inline bool File::Read(uint8(&outBuffer)[Size])
	{
		return Read((uint8*)outBuffer, Size);
	}

	template<uint64 Size>
	inline bool File::Read(char(&outBuffer)[Size])
	{
		return Read((uint8*)outBuffer, Size - 1);
	}

	inline bool File::Write(const uint8* inBuffer, uint64 count)
	{
		ionassert(m_Mode & EFileMode::Write, "Write access mode was not specified when opening the file.");

		return Write_Native(inBuffer, count);
	}

	inline bool File::Write(const char* inBuffer, uint64 count)
	{
		return Write((const uint8*)inBuffer, count);
	}

	inline bool File::Write(const String& inStr)
	{
		return Write(inStr.c_str(), inStr.size());
	}

	inline bool File::WriteLine(const char* inBuffer, uint64 count, ENewLineType newLineType)
	{
		ionassert(m_Mode & EFileMode::Write, "Write access mode was not specified when opening the file.");

		return WriteLine_Native(inBuffer, count, newLineType);
	}

	inline bool File::WriteLine(const String& inStr, ENewLineType newLineType)
	{
		return WriteLine(inStr.c_str(), inStr.size() + 1, newLineType);
	}

	template<uint64 Size>
	inline bool File::Write(const uint8(&inBuffer)[Size])
	{
		return Write((const uint8*)inBuffer, Size);
	}

	template<uint64 Size>
	inline bool File::Write(const char(&inBuffer)[Size])
	{
		return Write((const uint8*)inBuffer, Size - 1);
	}

	inline bool File::AddOffset(int64 count)
	{
		return AddOffset_Native(count);
	}

	inline bool File::SetOffset(int64 count)
	{
		return SetOffset_Native(count);
	}

	inline int64 File::GetOffset() const
	{
		return m_Offset;
	}

	inline const WString& File::GetExtension() const
	{
		return m_FileExtension;
	}

	inline bool File::IsOpen() const
	{
		return m_bOpen;
	}

	inline const WString& File::GetFullPath() const
	{
		return m_FilePath;
	}

#pragma endregion

// FilePath class -------------------------------------------------------------------------------

#pragma region FilePath

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

		void Back();

		bool Make(const WString& name);

		bool Rename(const WString& newName);

		bool Delete(bool bForce = false);

		/**
		 * @brief Get a relative path with a specified base directory.
		 * e.g. "C:/Programs/Base/Assets/Textures" (baseDir = "C:/Programs/Base") -> "Assets/Textures"
		 * 
		 * @param baseDir base directory
		 * @return FilePath relative file path
		 */
		FilePath AsRelativeFrom(const FilePath& baseDir) const;

		FilePath Fix() const;

		bool Exists() const;

		bool IsDirectory() const;

		bool IsFile() const;

		bool IsEmpty() const;

		FileList ListFiles() const;

		TShared<TTreeNode<FileInfo>> Tree() const;

		bool IsRelative() const;

		WString LastElement() const;

		void SetValidation(EFilePathValidation validation);

		const WString& ToString() const;

		static bool Exists(const wchar* path);

		static bool Exists(const WString& path);

		static bool IsDirectory(const wchar* path);

		static bool IsDirectory(const WString& path);

		static bool IsFile(const wchar* path);

		static bool IsFile(const WString& path);

		static FileList ListFiles(const wchar* path);

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

		operator WString() const;

	private:
		static TArray<WString> SplitPathName(const WString& path);
		static WStringView StripSlashes(const WString& name);

		TTreeNode<FileInfo>& Tree_Internal() const;

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

	inline bool FilePath::Make(const WString& name)
	{
		ionassert(File::IsFileNameLegal(name));

		return Make_Native(name.c_str());
	}

	inline bool FilePath::Rename(const WString& newName)
	{
		ionassert(File::IsFileNameLegal(newName));

		// @TODO: Implement this
		return true;
	}

	inline bool FilePath::Exists() const
	{
		return Exists(m_PathName.c_str());
	}

	inline bool FilePath::IsDirectory() const
	{
		return IsDirectory(m_PathName.c_str());
	}

	inline bool FilePath::IsFile() const
	{
		return Exists() && !IsDirectory();
	}

	inline bool FilePath::IsEmpty() const
	{
		return m_Path.empty();
	}

	inline WString FilePath::LastElement() const
	{
		return !m_Path.empty() ?
			m_Path.back() :
			L"";
	}

	inline void FilePath::SetValidation(EFilePathValidation validation)
	{
		m_bChecked = (bool)validation;
	}

	inline const WString& FilePath::ToString() const
	{
		return m_PathName;
	}

	inline bool FilePath::Exists(const wchar* path)
	{
		return Exists_Native(path);
	}

	inline bool FilePath::Exists(const WString& path)
	{
		return Exists(path.c_str());
	}

	inline bool FilePath::IsDirectory(const wchar* path)
	{
		return IsDirectory_Native(path);
	}

	inline bool FilePath::IsDirectory(const WString& path)
	{
		return IsDirectory(path.c_str());
	}

	inline bool FilePath::IsFile(const wchar* path)
	{
		return Exists(path) && !IsDirectory(path);
	}

	inline bool FilePath::IsFile(const WString& path)
	{
		return IsFile(path.c_str());
	}

	inline FilePath& FilePath::operator=(const WString& str)
	{
		Set(str);
		return *this;
	}

	inline FilePath& FilePath::operator=(WString&& str)
	{
		Set(str);
		return *this;
	}

	inline FilePath::operator WString() const
	{
		return ToString();
	}

#pragma endregion

}
