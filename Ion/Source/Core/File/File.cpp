#include "IonPCH.h"

#include "File.h"

namespace Ion
{
	File::File()
		: File(TEXT(""))
	{ }

	File::File(const WString& filename) :
		m_Filename(filename),
		m_Type(IO::FT_Text),
		m_Mode((IO::EFileMode)0),
		m_FileSize(-1),
		WriteNewLineType(IO::NLT_CRLF)
	{ }

	bool File::LoadToString(const WString& filename, String& outStr)
	{
		bool bResult;
		File* file = File::Create(filename);
		bResult = file->Open(IO::FM_Read);
		if (!bResult)
			return false;

		bResult = file->Read(outStr);
		if (!bResult)
			return false;

		file->Close();

		return true;
	}

	bool File::SetFilename(const WString& filename)
	{
		if (SetFilename_Impl(m_Filename))
		{
			m_Filename = filename;
			return true;
		}
		return false;
	}

	bool File::SetFilename_Impl(const WString& filename)
	{
		return true;
	}

	bool File::SetType(IO::EFileType type)
	{
		if (SetType_Impl(m_Type))
		{
			m_Type = type;
			return true;
		}
		return false;
	}

	bool File::SetType_Impl(IO::EFileType type)
	{
		return true;
	}
}
