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
	REGISTER_LOGGER(FileLogger, "Core::File", ELoggerFlags::None, ELogLevel::Warn);

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
			//DoNotOpen = Bitflag(5),
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

#pragma endregion

#pragma region FileList

	struct FileInfo
	{
		String Filename;
		String FullPath;
		int64 Size;
		bool bDirectory;

		FileInfo(const String& filename, const String& fullPath, int64 size, bool bDir) :
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

// FilePath class ---------------------------------------------------------------------------

#pragma region FilePath

	class ION_API FilePath
	{
	public:
		FilePath();
		FilePath(const String& path);
		FilePath(const WString& path);

		FilePath(const FilePath&) = default;
		FilePath(FilePath&&) noexcept = default;

		FilePath& Set(const String& path);
		FilePath& Set(const WString& path);
		FilePath& Set(const FilePath& path);

		FilePath& ChangeDirectory(const String& directory);
		FilePath& ChangeDirectory(const WString& directory);
		FilePath& ChangePath(const FilePath& path);

		FilePath& Back();

		/**
		 * @brief Creates a directory on disk with a given name.
		 * 
		 * @param name Name of the directory
		 * @return true if the directory has been made successfully.
		 */
		bool MkDir(const String& name);
		bool MkDir(const WString& name);

		bool Rename(const String& newName);
		bool Rename(const WString& newName);

		/**
		 * @brief Deletes the file / directory on disk.
		 * Doesn't delete the whole directory with files recursively, unless bForce is true.
		 * 
		 * @param bForce Whether to force delete the whole directory
		 * @return true if the file has been deleted
		 */
		bool Delete(bool bForce = false) const;

		/**
		 * @brief Get this FilePath as a relative path to a specified base directory.
		 * e.g. "C:/Programs/Base/Assets/Textures" (baseDir = "C:/Programs/Base") -> "Assets/Textures"
		 * This FilePath must be absolute.
		 * 
		 * @param baseDir base directory, must be absolute
		 * @return FilePath relative file path
		 */
		FilePath RelativeTo(const FilePath& baseDir) const;

		/**
		 * @brief Removes ".." and "." from the middle of the path, keeping the path intact.
		 * e.g. "Ion/Ion/../IonExample/Assets" -> "Ion/IonExample/Assets"
		 * 
		 * @return FilePath Fixed file path
		 */
		FilePath Fix() const;
		FilePath& Fix();

		/**
		 * @brief Checks if the file / directory exists on disk
		 */
		bool Exists() const;

		/**
		 * @brief Checks if the file path points to a directory on disk.
		 */
		bool IsDirectory() const;

		/**
		 * @brief Checks if the file path points to a file on disk.
		 */
		bool IsFile() const;

		bool IsEmpty() const;

		FileList ListFiles() const;

		TShared<TTreeNode<FileInfo>> Tree() const;

		bool IsRelative() const;
		bool IsAbsolute() const;

		String LastElement() const;
		WString LastElementW() const;

		StringView GetExtension() const;
		static StringView GetExtension(const StringView& name);

		const String& ToString() const;
		WString ToWString() const;

		static bool Exists(const wchar* path);
		static bool Exists(const String& path);
		static bool Exists(const WString& path);

		static bool IsDirectory(const wchar* path);
		static bool IsDirectory(const String& path);
		static bool IsDirectory(const WString& path);

		static bool IsFile(const wchar* path);
		static bool IsFile(const String& path);
		static bool IsFile(const WString& path);

		static bool IsDriveLetter(const String& drive);
		static bool IsDriveLetter(const WString& drive);

		static FileList ListFiles(const wchar* path);
		static FileList ListFiles(const String& path);

		FilePath& operator=(const FilePath&) = default;
		FilePath& operator=(FilePath&&) = default;
		FilePath& operator=(const String&);
		FilePath& operator=(String&&);
		FilePath& operator=(const WString&);
		FilePath& operator=(WString&&);

		FilePath& operator+=(const FilePath& path);
		FilePath& operator+=(const String& directory);
		FilePath& operator+=(const WString& directory);

		FilePath operator+(const FilePath& path) const;
		FilePath operator+(const String& directory) const;
		FilePath operator+(const WString& directory) const;

		FilePath& operator/=(const FilePath& path);
		FilePath& operator/=(const String& directory);
		FilePath& operator/=(const WString& directory);

		FilePath operator/(const FilePath& path) const;
		FilePath operator/(const String& directory) const;
		FilePath operator/(const WString& directory) const;

		bool operator==(const FilePath& path) const;
		bool operator==(const String& path) const;
		bool operator==(const WString& path) const;

		bool operator!=(const FilePath& path) const;
		bool operator!=(const String& path) const;
		bool operator!=(const WString& path) const;

		operator WString() const;
		operator String() const;

	private:
		static TArray<String> SplitPathName(const String& path);
		static StringView StripSlashes(const String& name);

		TTreeNode<FileInfo>& Tree_Internal() const;

		// Platform specific

		bool MkDir_Native(const wchar* name);
		bool Delete_Native() const;
		bool DeleteForce_Native() const;
		TArray<FileInfo> ListFiles_Native() const;
		static bool Exists_Native(const wchar* path);
		static bool IsDirectory_Native(const wchar* path);
		static bool IsDriveLetter_Native(const WString& drive);

	private:
		TArray<String> m_Path;
		
		mutable String m_PathName;
		void UpdatePathName() const;

		friend class File;
	};

#pragma endregion

// File class -------------------------------------------------------------------------------

#pragma region File

	class ION_API File
	{
	public:
		File(const FilePath& path);
		File(const String& filename);
		File(const WString& filename);

		Result<void, IOError, FileNotFoundError> Open(uint8 mode = EFileMode::Read);

		Result<void, IOError, FileNotFoundError> Delete();

		void Close();

		// Read functions

		Result<void, IOError> Read(uint8* outBuffer, uint64 count);
		Result<void, IOError> Read(char* outBuffer, uint64 count);
		Result<String, IOError> Read();

		Result<void, IOError> ReadLine(char* outBuffer, uint64 count);
		Result<String, IOError> ReadLine();

		template<uint64 Size>
		Result<void, IOError> Read(uint8(&outBuffer)[Size]);
		template<uint64 Size>
		Result<void, IOError> Read(char(&outBuffer)[Size]);

		static Result<String, IOError, FileNotFoundError> ReadToString(const FilePath& filePath);
		static Result<String, IOError, FileNotFoundError> ReadToString(const String& filePath);
		static Result<String, IOError, FileNotFoundError> ReadToString(const WString& filePath);

		// Write functions

		Result<void, IOError> Write(const uint8* inBuffer, uint64 count);
		Result<void, IOError> Write(const char* inBuffer, uint64 count);
		Result<void, IOError> Write(const String& inStr);

		Result<void, IOError> WriteLine(const char* inBuffer, uint64 count, ENewLineType newLineType = s_DefaultNewLineType);
		Result<void, IOError> WriteLine(const String& inStr, ENewLineType newLineType = s_DefaultNewLineType);

		template<uint64 Size>
		Result<void, IOError> Write(const uint8(&inBuffer)[Size]);
		template<uint64 Size>
		Result<void, IOError> Write(const char(&inBuffer)[Size]);

		// Other functions

		Result<void, IOError> AddOffset(int64 count);
		Result<void, IOError> SetOffset(int64 count);
		int64 GetOffset() const;

		int64 GetSize() const;

		StringView GetExtension() const;

		bool IsOpen() const;

		bool Exists() const;

		bool Eof() const;

		bool IsDirectory() const;

		const FilePath& GetFilePath() const;

		const String& GetFullPath() const;

		static bool IsFileNameLegal(const String& name);
		static bool IsFileNameLegal(const WString& name);

		~File();

		File(const File&) = delete;
		File(File&&) noexcept = default;

		File& operator=(const File&) = delete;
		File& operator=(File&&) = default;

	private:
		FilePath m_FilePath;
		int64 m_Offset;
		EFileType m_Type;
		uint8 m_Mode;
		bool m_bOpen;

	// Caches

		mutable int64 m_FileSize;
		/* Invalidates file size cache and retrieves the new size. */
		FORCEINLINE void UpdateFileSizeCache() const { m_FileSize = -1; GetSize(); }
		/* Sets the file size cache to the size specified. */
		FORCEINLINE void UpdateFileSizeCache(int64 newFileSize) const { m_FileSize = newFileSize; }

	// End of Caches

	// Platform specific

	private:
		Result<void, IOError, FileNotFoundError> Open_Native();
		Result<void, IOError, FileNotFoundError> Delete_Native();
		void Close_Native();

		Result<void, IOError> Read_Native(uint8* outBuffer, uint64 count);
		Result<void, IOError> ReadLine_Internal(char* outBuffer, uint64 count, uint64* outReadCount, bool* bOutOverflow);
		Result<void, IOError> ReadLine_Native(char* outBuffer, uint64 count);
		Result<String, IOError> ReadLine_Native();
		Result<void, IOError> Write_Native(const uint8* inBuffer, uint64 count);
		Result<void, IOError> WriteLine_Native(const char* inBuffer, uint64 count, ENewLineType newLineType);

		Result<void, IOError> AddOffset_Native(int64 count);
		Result<void, IOError> SetOffset_Native(int64 count);

		void SetNativePointer_Native();

		void* m_NativePointer;

		static const ENewLineType s_DefaultNewLineType;

#if ION_PLATFORM_WINDOWS
		static constexpr wchar s_IllegalCharactersW[] = L"/\\*<>|?:\"";
		static constexpr char  s_IllegalCharacters[]  =  "/\\*<>|?:\"";
#else
		static constexpr wchar s_IllegalCharactersW[] = L"";
		static constexpr char  s_IllegalCharacters[]  =  "";
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

#pragma endregion

// FilePath implementation ------------------------------------------------------------------

#pragma region FilePath_Impl

	inline bool FilePath::MkDir(const String& name)
	{
		ionassert(File::IsFileNameLegal(name));

		return MkDir(StringConverter::StringToWString(name));
	}

	inline bool FilePath::MkDir(const WString& name)
	{
		ionassert(File::IsFileNameLegal(name));

		return MkDir_Native(name.c_str());
	}

	inline bool FilePath::Rename(const String& newName)
	{
		ionassert(File::IsFileNameLegal(newName));

		// @TODO: Implement this
		return true;
	}

	inline bool FilePath::Rename(const WString& newName)
	{
		return Rename(StringConverter::WStringToString(newName));
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

	inline String FilePath::LastElement() const
	{
		return !m_Path.empty() ?
			m_Path.back() :
			"";
	}

	inline WString FilePath::LastElementW() const
	{
		return !m_Path.empty() ?
			StringConverter::StringToWString(m_Path.back()) :
			L"";
	}

	inline StringView FilePath::GetExtension() const
	{
		return GetExtension(m_PathName);
	}

	inline StringView FilePath::GetExtension(const StringView& name)
	{
		ionassert(!name.empty());

		uint64 dotIndex = name.find_last_of(L'.');

		if (dotIndex == String::npos)
			return "";
		
		return name.substr(dotIndex);
	}

	inline const String& FilePath::ToString() const
	{
		return m_PathName;
	}

	inline WString FilePath::ToWString() const
	{
		return StringConverter::StringToWString(m_PathName);
	}

	inline bool FilePath::Exists(const wchar* path)
	{
		return Exists_Native(path);
	}

	inline bool FilePath::Exists(const String& path)
	{
		return Exists(StringConverter::StringToWString(path));
	}

	inline bool FilePath::Exists(const WString& path)
	{
		return Exists(path.c_str());
	}

	inline bool FilePath::IsDirectory(const wchar* path)
	{
		return IsDirectory_Native(path);
	}

	inline bool FilePath::IsDirectory(const String& path)
	{
		return IsDirectory(StringConverter::StringToWString(path));
	}

	inline bool FilePath::IsDirectory(const WString& path)
	{
		return IsDirectory(path.c_str());
	}

	inline bool FilePath::IsFile(const wchar* path)
	{
		return Exists(path) && !IsDirectory(path);
	}

	inline bool FilePath::IsFile(const String& path)
	{
		return IsFile(StringConverter::StringToWString(path));
	}

	inline bool FilePath::IsFile(const WString& path)
	{
		return IsFile(path.c_str());
	}

	inline bool FilePath::IsDriveLetter(const String& drive)
	{
		return IsDriveLetter(StringConverter::StringToWString(drive));
	}

	inline bool FilePath::IsDriveLetter(const WString& drive)
	{
		return IsDriveLetter_Native(drive);
	}
	
	inline FilePath& FilePath::operator=(const String& str)
	{
		Set(str);
		return *this;
	}

	inline FilePath& FilePath::operator=(String&& str)
	{
		Set(Move(str));
		return *this;
	}

	inline FilePath& FilePath::operator=(const WString& str)
	{
		Set(str);
		return *this;
	}

	inline FilePath& FilePath::operator=(WString&& str)
	{
		Set(Move(str));
		return *this;
	}

	inline FilePath::operator String() const
	{
		return ToString();
	}

	inline FilePath::operator WString() const
	{
		return ToWString();
	}

#pragma endregion

// File implementation ----------------------------------------------------------------------

#pragma region File_Impl

	inline Result<void, IOError> File::Read(uint8* outBuffer, uint64 count)
	{
		ionassert(m_bOpen);
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		return Read_Native(outBuffer, count)
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&] { FileLogger.Debug("Read [@TODO] bytes from file \"{}\".", m_FilePath.ToString()); });
	}

	inline Result<void, IOError> File::Read(char* outBuffer, uint64 count)
	{
		return Read((uint8*)outBuffer, count);
	}

	inline Result<void, IOError> File::ReadLine(char* outBuffer, uint64 count)
	{
		ionassert(m_bOpen);
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		return ReadLine_Native(outBuffer, count)
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&] { FileLogger.Debug("Read [@TODO] bytes from file \"{}\".", m_FilePath.ToString()); });
	}

	inline Result<String, IOError> File::ReadLine()
	{
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		return ReadLine_Native()
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&](const String& value) { FileLogger.Debug("Read [@TODO] bytes from file \"{}\".", m_FilePath.ToString()); });
	}

	template<uint64 Size>
	inline Result<void, IOError> File::Read(uint8(&outBuffer)[Size])
	{
		return Read((uint8*)outBuffer, Size);
	}

	template<uint64 Size>
	inline Result<void, IOError> File::Read(char(&outBuffer)[Size])
	{
		return Read((uint8*)outBuffer, Size - 1);
	}

	inline Result<void, IOError> File::Write(const uint8* inBuffer, uint64 count)
	{
		ionassert(m_Mode & EFileMode::Write, "Write access mode was not specified when opening the file.");

		return Write_Native(inBuffer, count)
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&] { FileLogger.Debug("Written [@TODO] bytes to file \"{}\".", m_FilePath.ToString()); });
	}

	inline Result<void, IOError> File::Write(const char* inBuffer, uint64 count)
	{
		return Write((const uint8*)inBuffer, count);
	}

	inline Result<void, IOError> File::Write(const String& inStr)
	{
		return Write(inStr.c_str(), inStr.size());
	}

	inline Result<void, IOError> File::WriteLine(const char* inBuffer, uint64 count, ENewLineType newLineType)
	{
		ionassert(m_Mode & EFileMode::Write, "Write access mode was not specified when opening the file.");

		return WriteLine_Native(inBuffer, count, newLineType)
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&] { FileLogger.Debug("Written [@TODO] bytes to file \"{}\".", m_FilePath.ToString()); });
	}

	inline Result<void, IOError> File::WriteLine(const String& inStr, ENewLineType newLineType)
	{
		return WriteLine(inStr.c_str(), inStr.size() + 1, newLineType);
	}

	template<uint64 Size>
	inline Result<void, IOError> File::Write(const uint8(&inBuffer)[Size])
	{
		return Write((const uint8*)inBuffer, Size);
	}

	template<uint64 Size>
	inline Result<void, IOError> File::Write(const char(&inBuffer)[Size])
	{
		return Write((const uint8*)inBuffer, Size - 1);
	}

	inline Result<void, IOError> File::AddOffset(int64 count)
	{
		return AddOffset_Native(count)
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&] { FileLogger.Debug("Moved pointer by {} bytes in file \"{}\".", count, m_FilePath.ToString()); });
	}

	inline Result<void, IOError> File::SetOffset(int64 count)
	{
		return SetOffset_Native(count)
			.Err([](Error& err) { FileLogger.Error(err.Message); })
			.Ok([&] { FileLogger.Debug("Pointer has been set to {} bytes in file \"{}\".", count, m_FilePath.ToString()); });;
	}

	inline int64 File::GetOffset() const
	{
		return m_Offset;
	}

	inline StringView File::GetExtension() const
	{
		return GetFilePath().GetExtension();
	}

	inline bool File::IsOpen() const
	{
		return m_bOpen;
	}

	inline bool File::Exists() const
	{
		return FilePath::Exists(StringConverter::StringToWString(m_FilePath));
	}

	inline bool File::IsDirectory() const
	{
		return FilePath::IsDirectory(StringConverter::StringToWString(m_FilePath));
	}

	inline const FilePath& File::GetFilePath() const
	{
		return m_FilePath;
	}

	inline const String& File::GetFullPath() const
	{
		return m_FilePath.ToString();
	}

#pragma endregion

}
