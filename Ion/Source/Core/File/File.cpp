#include "IonPCH.h"

#include "File.h"

namespace Ion
{
	FileOld::FileOld()
		: FileOld(TEXT(""))
	{ }

	FileOld::FileOld(const WString& filename) :
		m_Filename(filename),
		m_Type(IO::FT_Text),
		m_Mode((IO::EFileMode)0),
		m_FileSize(-1),
		WriteNewLineType(IO::NLT_CRLF)
	{ }

	bool FileOld::LoadToString(const WString& filename, String& outStr)
	{
		bool bResult;
		FileOld* file = FileOld::Create(filename);
		bResult = file->Open(IO::FM_Read);
		if (!bResult)
			return false;

		bResult = file->Read(outStr);
		if (!bResult)
			return false;

		file->Close();

		return true;
	}

	bool FileOld::SetFilename(const WString& filename)
	{
		if (SetFilename_Impl(m_Filename))
		{
			m_Filename = filename;
			return true;
		}
		return false;
	}

	bool FileOld::SetFilename_Impl(const WString& filename)
	{
		return true;
	}

	bool FileOld::SetType(IO::EFileType type)
	{
		if (SetType_Impl(m_Type))
		{
			m_Type = type;
			return true;
		}
		return false;
	}

	bool FileOld::SetType_Impl(IO::EFileType type)
	{
		return true;
	}

	//-----------------------------------------------
	// New File API ---------------------------------
	//-----------------------------------------------

	File::File(const FilePath& path, uint8 mode)
		: File(path.ToString(), mode)
	{ }

	File::File(const WString& filename, uint8 mode) :
		m_Filename(filename),
		m_Type(EFileType::Text),
		m_Mode(mode),
		m_bOpen(false),
		m_Offset(0),
		m_FileSize(0)
	{
		ionassertnd(Open(filename, mode));

		UpdateFileExtensionCache();
	}

	File::~File()
	{
		if (IsOpen())
			Close();
	}

	bool File::Open(const FilePath& path, uint8 mode)
	{
		return Open(path.ToString(), mode);
	}

	bool File::Open(const WString& filename, uint8 mode)
	{
		ionassert(!IsOpen());
		ionassert(!filename.empty(), "The filename cannot be empty.");
		ionassert(mode & (EFileMode::Read | EFileMode::Write), "File mode has to have at least one of Read and Write flags set.");
		ionassert(!(mode & EFileMode::Read && !(mode & EFileMode::Write) && mode & EFileMode::Reset),
			"You cannot reset the file, if you only want to read from it.");
		ionassert(!(mode & EFileMode::Read && !(mode & EFileMode::Write) && mode & EFileMode::CreateNew),
			"Creating a new file, if it is only going to be read, is redundant.");

		return Open_Native();
	}

	bool File::Delete(const FilePath& path)
	{
		return Delete_Native(path.ToString());
	}

	void File::Close()
	{
		ionassert(IsOpen());

		m_Mode = 0;
		m_Offset = 0;
		m_FileSize = 0;

		Close_Native();
	}

	bool File::Read(String& outStr)
	{
		ionassert(m_Mode & IO::FM_Read, "Read access mode was not specified when opening the file.");

		int64 count = GetSize();
		outStr.resize(count);

		return Read(outStr.data(), count);
	}

	void File::UpdateFileExtensionCache() const
	{
		ionassert(!m_Filename.empty());

		uint64 dotIndex = m_Filename.find_last_of(L'.');

		if (dotIndex == WString::npos)
		{
			m_FileExtension = L"";
		}
		else
		{
			WString extension = m_Filename.substr(dotIndex + 1, (size_t)-1);

			std::transform(extension.begin(), extension.end(), extension.begin(), [](wchar ch) { return std::tolower(ch); });
			m_FileExtension = extension;
		}
	}

	bool File::EndOfFile() const
	{
		return m_Offset >= GetSize();
	}

	bool File::IsFileNameLegal(const WString& name)
	{
		if (name.empty())
		{
			return false;
		}

		for (int32 i = 0; i < name.size(); ++i)
		{
			if (IsAnyOf(name[i], s_IllegalCharacters))
			{
				return false;
			}
		}
		return true;
	}

#if ION_DEBUG
	bool File::s_GlobalDebugLog = false;
#endif

	// -----------------------------------------------------
	// FilePath: -------------------------------------------
	// -----------------------------------------------------

	FilePath::FilePath(const WString& path, EFilePathValidation validation) :
		m_bChecked((bool)validation)
	{
		Set(path);
	}

	FilePath::FilePath(const FilePath& path, EFilePathValidation validation) :
		m_bChecked((bool)validation)
	{
		Set(path);
	}

	void FilePath::Set(const WString& path)
	{
		ionassert(path.size() < MaxPathLength);

		m_Path = SplitPathName(path);
		UpdatePathName();
	}

	void FilePath::Set(const FilePath& path)
	{
		m_Path = path.m_Path;
		m_PathName = path.m_PathName;
	}

	bool FilePath::ChangeDirectory(const WString& directory)
	{
		WString strippedName(StripSlashes(directory));
		ionassert(File::IsFileNameLegal(strippedName));

		if (strippedName == L".")
		{
			return true;
		}

		if (strippedName == L"..")
		{
			Back();
		}
		else
		{
			WString newPath = m_PathName.empty() ? strippedName : m_PathName + L"/" + strippedName;
			if (m_bChecked && !Exists(newPath.c_str()))
			{
				LOG_ERROR(L"Path \"{0}\" does not exist!", newPath);
				ionassert(false, "This should not happen when using validated path operations.");
				return false;
			}
			// Add the directory to the end instead of calling UpdatePathName
			m_PathName = newPath;
			m_Path.emplace_back(Move(strippedName));
		}

		return true;
	}

	bool FilePath::ChangePath(const FilePath& path)
	{
		WString newPath;
		bool bChanged = false;
		for (const WString& dir : path.m_Path)
		{
			newPath = m_PathName.empty() ? dir : m_PathName + L"/" + dir;
			if (m_bChecked && !Exists(newPath.c_str()))
			{
				LOG_ERROR(L"Path \"{0}\" does not exist!", newPath);
				ionassert(false, "This should not happen when using validated path operations.");
				break;
			}
			
			m_Path.push_back(dir);
			m_PathName = newPath;
			bChanged = true;
		}

		return bChanged;
	}

	bool FilePath::Delete(bool bForce)
	{
		if (bForce)
		{
			return DeleteForce_Native();
		}

		if (ListFiles().IsEmpty())
		{
			return Delete_Native();
		}

		return false;
	}

	bool File::ReadToString(const FilePath& filePath, String& outString)
	{
		ionassert(filePath.Exists() && filePath.IsFile(), "The file does not exist or is a directory.");

		File file(filePath, EFileMode::Read);
		return file.Read(outString);
	}

	FileList FilePath::ListFiles() const
	{
		ionassert(Exists());

		TArray<FileInfo> files = ListFiles_Native();
		FileList list(Move(files));

		return list;
	}

	bool FilePath::IsRelative() const
	{
		// @TODO: Implement this
		return true;
	}

	FileList FilePath::ListFiles(const wchar* path)
	{
		return FilePath(path).ListFiles();
	}

	FilePath& FilePath::operator+=(const FilePath& path)
	{
		ChangePath(path);
		return *this;
	}

	FilePath& FilePath::operator+=(const WString& directory)
	{
		ChangePath(directory);
		return *this;
	}

	FilePath FilePath::operator+(const FilePath& path) const
	{
		FilePath newPath = *this;
		newPath.ChangePath(path);
		return newPath;
	}

	FilePath FilePath::operator+(const WString& directory) const
	{
		FilePath newPath = *this;
		newPath.ChangePath(directory);
		return newPath;
	}

	TArray<WString> FilePath::SplitPathName(const WString& path)
	{
#pragma warning(disable:6255)
#pragma warning(disable:6386)
		TArray<WString> pathArray;

		uint64 size = path.size();
		wchar* segment = (wchar*)_alloca((size + 1) * sizeof(wchar));
		memset(segment, 0, (size + 1) * sizeof(wchar));

		// This flag prevents adding empty directories (multiple slashes next to each other) to the list.
		bool bEmptySegment = true;
		int32 segmentIt = 0;
		for (int32 i = 0; i < size; ++i)
		{
			wchar current = path[i];
			if (IsAnyOf(current, L'/', L'\\'))
			{
				if (!bEmptySegment)
				{
					pathArray.emplace_back<WString>(segment);
					memset(segment, 0, (size + 1) * sizeof(wchar));
				}

				segmentIt = 0;
				bEmptySegment = true;
			}
			else
			{
				bEmptySegment = false;
				segment[segmentIt++] = current;
			}
		}
		// Add the last element if there wasn't a slash at the end
		if (!bEmptySegment)
		{
			ionassert(File::IsFileNameLegal(segment));
			pathArray.emplace_back<WString>(segment);
		}

		return pathArray;
	}

	WStringView FilePath::StripSlashes(const WString& name)
	{
#pragma warning(disable:26451)
		uint64 size = name.size();
		int32 start = 0;
		int32 end = 0;
		bool bName = false;
		for (int32 i = 0; i < size; ++i)
		{
			if (IsAnyOf(name[i], L'/', L'\\'))
			{
				if (bName)
					break;

				start++;
			}
			else
			{
				bName = true;
				end = i + 1;
			}
		}

		return WStringView(name).substr(start, end - start);
	}

	void FilePath::UpdatePathName() const
	{
		m_PathName.clear();
		uint32 last = (uint32)m_Path.size() - 1;
		uint32 current = 0;
		for (const WString& dir : m_Path)
		{
			uint64 sizeInWords = dir.size() + 2;
			uint64 size = sizeInWords * sizeof(wchar);
			wchar* segment = (wchar*)_alloca(size);
			memset(segment, 0, size);

			wcscpy_s(segment, sizeInWords, dir.c_str());
			if (current != last)
			{
				wcscat_s(segment, sizeInWords, L"/");
			}

			m_PathName += segment;
			current++;
		}
	}
}
