#include "IonPCH.h"

#include "File.h"

namespace Ion
{
	File::File(const FilePath& path, uint8 mode)
		: File(path.ToString(), mode)
	{ }

	File::File(const WString& filename, uint8 mode) :
		m_FilePath(filename),
		m_Type(EFileType::Text),
		m_Mode(mode),
		m_bOpen(false),
		m_Offset(0),
		m_FileSize(0)
	{
		ionassert(!filename.empty(), "The filename cannot be empty.");

		UpdateFileExtensionCache();

		if (!(m_Mode & EFileMode::DoNotOpen))
		{
			ionverify(Open(mode));
		}
	}

	File::~File()
	{
		if (IsOpen())
			Close();
	}

	bool File::Open(uint8 mode)
	{
		ionassert(!IsOpen());
		ionassert(mode & (EFileMode::Read | EFileMode::Write), "File mode has to have at least one of Read and Write flags set.");
		ionassert(!(mode & EFileMode::Read && !(mode & EFileMode::Write) && mode & EFileMode::Reset),
			"You cannot reset the file, if you only want to read from it.");
		ionassert(!(mode & EFileMode::Read && !(mode & EFileMode::Write) && mode & EFileMode::CreateNew),
			"Creating a new file, if it is only going to be read, is redundant.");

		m_Mode = mode;
		m_bOpen = true;

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
		m_bOpen = false;

		Close_Native();
	}

	bool File::ReadToString(const FilePath& filePath, String& outString)
	{
		return ReadToString(filePath.ToString(), outString);
	}

	bool File::ReadToString(const WString& filePath, String& outString)
	{
		if (!(FilePath::Exists(filePath) && FilePath::IsFile(filePath)))
		{
			LOG_ERROR(L"The file \"{0}\" does not exist or is a directory.", filePath);
			return false;
		}

		File file(filePath, EFileMode::Read);
		return file.Read(outString);
	}

	bool File::Read(String& outStr)
	{
		ionassert(m_Mode & EFileMode::Read, "Read access mode was not specified when opening the file.");

		int64 count = GetSize();
		outStr.resize(count);

		return Read(outStr.data(), count);
	}

	void File::UpdateFileExtensionCache() const
	{
		ionassert(!m_FilePath.empty());

		uint64 dotIndex = m_FilePath.find_last_of(L'.');

		if (dotIndex == WString::npos)
		{
			m_FileExtension = L"";
		}
		else
		{
			WString extension = m_FilePath.substr(dotIndex + 1, (size_t)-1);

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

	FilePath::FilePath() :
		m_bChecked(false)
	{
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

	void FilePath::Back()
	{
		if (m_Path.empty())
		{
			m_Path.push_back(L"..");
		}
		else
		{
			bool bOnlyBack = true;
			for (const WString& dir : m_Path)
			{
				if (dir != L"..")
				{
					bOnlyBack = false;
					break;
				}
			}
			if (!bOnlyBack)
				m_Path.pop_back();
			else
				m_Path.push_back(L"..");
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

	FilePath FilePath::AsRelativeFrom(const FilePath& baseDir) const
	{
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
			LOG_ERROR(L"Cannot find base directory \"{0}\" in path \"{1}\".", baseDir.ToString(), m_PathName);
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
		fixed.Set(m_Path.at(0));
		for (auto it = m_Path.begin() + 1; it != m_Path.end(); ++it)
		{
			fixed.ChangeDirectory(*it);
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

	bool FilePath::operator==(const FilePath& path) const
	{
		return m_PathName == path.m_PathName;
	}

	bool FilePath::operator==(const WString& path) const
	{
		return m_PathName == path;
	}

	bool FilePath::operator!=(const FilePath& path) const
	{
		return m_PathName != path.m_PathName;
	}

	bool FilePath::operator!=(const WString& path) const
	{
		return m_PathName == path;
	}

	TArray<WString> FilePath::SplitPathName(const WString& path)
	{
#pragma warning(disable:6255)
#pragma warning(disable:6386)
		// @TODO: Make a string split thingy instead of this nonsense

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
			//ionassert(File::IsFileNameLegal(segment));
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
			return file.bDirectory && file.Filename != L"." && file.Filename != L"..";
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
