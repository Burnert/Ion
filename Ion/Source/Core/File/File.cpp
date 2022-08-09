#include "IonPCH.h"

#include "File.h"

namespace Ion
{
	File::File(const FilePath& path)
		: File(path.ToString())
	{ }

	File::File(const String& filename) :
		m_FilePath(filename),
		m_Type(EFileType::Text),
		m_Mode(EFileMode::Read),
		m_bOpen(false),
		m_Offset(0),
		m_FileSize(0)
	{
		ionassert(!filename.empty(), "The filename cannot be empty.");

		SetNativePointer_Native();
	}

	File::File(const WString& filename) :
		File(StringConverter::WStringToString(filename))
	{
	}

	File::~File()
	{
		if (IsOpen())
			Close();
	}

	Result<void, IOError, FileNotFoundError> File::Open(uint8 mode)
	{
		ionassert(!IsOpen());
		ionassert(mode & (EFileMode::Read | EFileMode::Write), "File mode has to have at least one of Read and Write flags set.");
		ionassert(!(mode & EFileMode::Read && !(mode & EFileMode::Write) && mode & EFileMode::Reset),
			"You cannot reset the file, if you only want to read from it.");
		ionassert(!(mode & EFileMode::Read && !(mode & EFileMode::Write) && mode & EFileMode::CreateNew),
			"Creating a new file, if it is only going to be read, is redundant.");

		m_Mode = mode;

		return Open_Native();
	}

	bool File::Delete(const FilePath& path)
	{
		return Delete_Native(path.ToWString().c_str());
	}

	void File::Close()
	{
		ionassert(IsOpen());

		m_Mode = 0;
		m_Offset = 0;
		m_FileSize = 0;
		m_bOpen = false;

		Close_Native();
	}

	Result<String, IOError, FileNotFoundError> File::ReadToString(const FilePath& filePath)
	{
		return ReadToString(filePath.ToWString());
	}

	Result<String, IOError, FileNotFoundError> File::ReadToString(const String& filePath)
	{
		return ReadToString(StringConverter::StringToWString(filePath));
	}

	Result<String, IOError, FileNotFoundError> File::ReadToString(const WString& filePath)
	{
		if (!(FilePath::Exists(filePath) && FilePath::IsFile(filePath)))
		{
			LOG_ERROR(L"The file \"{}\" does not exist or is a directory.", filePath);
			ionthrow(FileNotFoundError, L"The file \"{}\" does not exist or is a directory.", filePath);
		}

		File file(filePath);
		file.Open();
		ionmatchresult(file.Read(),
			mfwdthrowall
			melse return R.Unwrap();
		);
	}

	Result<String, IOError> File::Read()
	{
		ionassert(m_bOpen);
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		int64 count = GetSize();
		String outStr(count, 0);

		fwdthrowall(Read(outStr.data(), count));
		return outStr;
	}

	bool File::EndOfFile() const
	{
		return m_Offset >= GetSize();
	}

	bool File::IsFileNameLegal(const String& name)
	{
		if (name.empty())
			return false;

		for (const char& c : name)
		{
			if (IsAnyOf(c, s_IllegalCharacters))
				return false;
		}
		return true;
	}

	bool File::IsFileNameLegal(const WString& name)
	{
		if (name.empty())
			return false;

		for (const wchar& c : name)
		{
			if (IsAnyOf(c, s_IllegalCharactersW))
				return false;
		}
		return true;
	}

#if ION_DEBUG
	bool File::s_GlobalDebugLog = false;
#endif

	// -----------------------------------------------------
	// FilePath: -------------------------------------------
	// -----------------------------------------------------

	FilePath::FilePath() :
		m_bChecked(false)
	{
	}

	FilePath::FilePath(const String& path, EFilePathValidation validation) :
		m_bChecked((bool)validation)
	{
		Set(path);
	}

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

	void FilePath::Set(const String& path)
	{
		m_Path = SplitPathName(path);
		UpdatePathName();
	}

	void FilePath::Set(const WString& path)
	{
		//ionassert(path.size() < MaxPathLength);

		Set(StringConverter::WStringToString(path));
	}

	void FilePath::Set(const FilePath& path)
	{
		m_Path = path.m_Path;
		m_PathName = path.m_PathName;
	}

	bool FilePath::ChangeDirectory(const String& directory)
	{
		String strippedName(StripSlashes(directory));
		ionassert(File::IsFileNameLegal(strippedName) || (m_Path.empty() && IsDriveLetter(strippedName)));

		if (strippedName == ".")
		{
			return true;
		}

		if (strippedName == "..")
		{
			Back();
			return true;
		}

		String newPath = m_PathName.empty() ? strippedName : m_PathName + "/" + strippedName;
		if (m_bChecked && !Exists(newPath.c_str()))
		{
			LOG_ERROR("Path \"{0}\" does not exist!", newPath);
			ionbreak("This should not happen when using validated path operations.");
			return false;
		}
		// Add the directory to the end instead of calling UpdatePathName
		m_PathName = newPath;
		m_Path.emplace_back(Move(strippedName));

		return true;
	}

	bool FilePath::ChangeDirectory(const WString& directory)
	{
		return ChangeDirectory(StringConverter::WStringToString(directory));
	}

	bool FilePath::ChangePath(const FilePath& path)
	{
		String newPath;
		bool bChanged = false;
		for (const String& dir : path.m_Path)
		{
			newPath = m_PathName.empty() ? dir : m_PathName + "/" + dir;
			if (m_bChecked && !Exists(newPath.c_str()))
			{
				LOG_ERROR("Path \"{0}\" does not exist!", newPath);
				ionbreak("This should not happen when using validated path operations.");
				break;
			}
			
			m_Path.push_back(dir);
			m_PathName = newPath;
			bChanged = true;
		}

		return bChanged;
	}

	void FilePath::Back()
	{
		while (!m_Path.empty() && m_Path.back() == ".")
			m_Path.pop_back();

		if (m_Path.empty() ||
			std::all_of(m_Path.begin(), m_Path.end(), [](const String& dir) { return dir == ".."; }))
		{
			m_Path.push_back("..");
		}
		else
		{
			ionassert(m_Path.size() > 1 || !IsDriveLetter(m_Path[0]),
				"Cannot go back if the only directory is a drive letter.");
			m_Path.pop_back();
		}
		UpdatePathName();
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

	FilePath FilePath::RelativeTo(const FilePath& baseDir) const
	{
		ionassert(IsAbsolute());
		ionassert(baseDir.IsAbsolute());

		FilePath relative;

		auto itBaseDir = baseDir.m_Path.begin();
		auto itThisDir = m_Path.begin();

		while (
			itThisDir != m_Path.end() &&
			itBaseDir != baseDir.m_Path.end() &&
			*itBaseDir == *itThisDir)
		{
			itBaseDir++;
			itThisDir++;
		}

		if (itBaseDir != baseDir.m_Path.end())
		{
			LOG_ERROR("Cannot find base directory \"{0}\" in path \"{1}\".", baseDir.ToString(), m_PathName);
			return relative;
		}

		// Not the fastest approach, but it should't be a problem.
		while (itThisDir != m_Path.end())
		{
			relative.ChangeDirectory(*itThisDir++);
		}

		return relative;
	}

	FilePath FilePath::Fix() const
	{
		if (IsEmpty())
			return FilePath();

		FilePath fixed;
		for (const String& dir : m_Path)
		{
			fixed.ChangeDirectory(dir);
		}
		return fixed;
	}

	FileList FilePath::ListFiles() const
	{
		ionassert(Exists());

		TArray<FileInfo> files = ListFiles_Native();
		FileList list(Move(files));

		return list;
	}

	TShared<TTreeNode<FileInfo>> FilePath::Tree() const
	{
		return MakeShareable(&Tree_Internal());
	}

	bool FilePath::IsRelative() const
	{
		// Empty path is like ".", so it's relative.
		if (IsEmpty())
			return true;

		return !IsDriveLetter_Native(StringConverter::StringToWString(m_Path[0]));
	}

	bool FilePath::IsAbsolute() const
	{
		return !IsRelative();
	}

	FileList FilePath::ListFiles(const wchar* path)
	{
		return FilePath(path).ListFiles();
	}

	FileList FilePath::ListFiles(const String& path)
	{
		return FilePath(path).ListFiles();
	}

	FilePath& FilePath::operator+=(const FilePath& path)
	{
		ChangePath(path);
		return *this;
	}

	FilePath& FilePath::operator+=(const String& directory)
	{
		ChangePath(directory);
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

	FilePath FilePath::operator+(const String& directory) const
	{
		FilePath newPath = *this;
		newPath.ChangePath(directory);
		return newPath;
	}

	FilePath FilePath::operator+(const WString& directory) const
	{
		FilePath newPath = *this;
		newPath.ChangePath(directory);
		return newPath;
	}

	bool FilePath::operator==(const FilePath& path) const
	{
		return m_PathName == path.m_PathName;
	}

	bool FilePath::operator==(const String& path) const
	{
		return m_PathName == path;
	}

	bool FilePath::operator==(const WString& path) const
	{
		return m_PathName == StringConverter::WStringToString(path);
	}

	bool FilePath::operator!=(const FilePath& path) const
	{
		return m_PathName != path.m_PathName;
	}

	bool FilePath::operator!=(const String& path) const
	{
		return m_PathName == path;
	}

	bool FilePath::operator!=(const WString& path) const
	{
		return m_PathName == StringConverter::WStringToString(path);
	}

	TArray<String> FilePath::SplitPathName(const String& path)
	{
#pragma warning(disable:6255)
#pragma warning(disable:6386)
		// @TODO: Make a string split thingy instead of this nonsense

		TArray<String> pathArray;

		String segment;
		segment.reserve(path.size());

		// This flag prevents adding empty directories (multiple slashes next to each other) to the list.
		bool bEmptySegment = true;
		//int32 segmentIt = 0;
		for (int32 i = 0; i < path.size(); ++i)
		{
			char current = path[i];
			if (IsAnyOf(current, L'/', L'\\'))
			{
				if (!bEmptySegment)
				{
					ionassert(File::IsFileNameLegal(segment) || (pathArray.empty() && IsDriveLetter(segment)));
					pathArray.emplace_back(segment);
					segment.clear();
				}

				//segmentIt = 0;
				bEmptySegment = true;
			}
			else
			{
				bEmptySegment = false;
				segment += current;
			}
		}
		// Add the last element if there wasn't a slash at the end
		if (!bEmptySegment)
		{
			ionassert(File::IsFileNameLegal(segment) || (pathArray.empty() && IsDriveLetter(segment)));
			pathArray.emplace_back(segment);
		}

		return pathArray;
	}

	StringView FilePath::StripSlashes(const String& name)
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

		return StringView(name).substr(start, end - start);
	}

	TTreeNode<FileInfo>& FilePath::Tree_Internal() const
	{
		ionassert(Exists() && IsDirectory());

		FileInfo thisDir = { LastElement(), m_PathName, 0, true };

		FileList thisDirList = ListFiles();
		// Filter the files
		FileList thisDirFiles = thisDirList.Filter([](const FileInfo& file)
		{
			return !file.bDirectory;
		});
		// Filter the directories
		FileList thisDirDirs = thisDirList.Filter([](const FileInfo& file)
		{
			return file.bDirectory && file.Filename != "." && file.Filename != "..";
		});

		TTreeNode<FileInfo>& thisDirNode = TTreeNode<FileInfo>::Make(thisDir);

		// Insert the files
		for (const FileInfo& file : thisDirFiles)
		{
			thisDirNode.Insert(TTreeNode<FileInfo>::Make(file));
		}

		// Recursively get the sub-directories.
		for (const FileInfo& dir : thisDirDirs)
		{
			FilePath subDir = *this + dir.Filename;
			thisDirNode.Insert(subDir.Tree_Internal());
		}

		return thisDirNode;
	}

	WString FilePath::GetPathW() const
	{
		return StringConverter::StringToWString(m_PathName);
	}

	void FilePath::UpdatePathName() const
	{
		m_PathName.clear();
		for (auto it = m_Path.begin(); it != m_Path.end(); ++it)
		{
			m_PathName += *it;
			if (it != m_Path.end() - 1)
				m_PathName += "/";
		}
	}
}
