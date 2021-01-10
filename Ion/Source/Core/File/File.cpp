#include "IonPCH.h"

#include "File.h"

namespace Ion
{
	FileBase::FileBase()
		: FileBase(TEXT(""))
	{ }

	FileBase::FileBase(const std::wstring& filename)
		: m_Filename(filename), m_Type(IO::FT_Text)
	{ }

	bool FileBase::SetFilename(const std::wstring& filename)
	{
		if (SetFilename_Impl(m_Filename))
		{
			m_Filename = filename;
			return true;
		}
		return false;
	}

	bool FileBase::SetFilename_Impl(const std::wstring& filename)
	{
		return true;
	}

	bool FileBase::SetType(IO::FileType type)
	{
		if (SetType_Impl(m_Type))
		{
			m_Type = type;
			return true;
		}
		return false;
	}

	bool FileBase::SetType_Impl(IO::FileType type)
	{
		return true;
	}
}
