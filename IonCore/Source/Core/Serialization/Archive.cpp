#include "Core/CorePCH.h"

#include "Archive.h"
#include "XMLArchive.h"

namespace Ion
{
	void XMLArchiveAdapter::EnterNode(const String& name)
	{
		if (XMLArchive* ar = AsXMLArchive())
			ar->EnterNode(name);
	}

	bool XMLArchiveAdapter::TryEnterNode(const String& name)
	{
		if (XMLArchive* ar = AsXMLArchive())
			return ar->TryEnterNode(name);
		return false;
	}

	void XMLArchiveAdapter::ExitNode()
	{
		if (XMLArchive* ar = AsXMLArchive())
			ar->ExitNode();
	}

	void XMLArchiveAdapter::EnterAttribute(const String& name)
	{
		if (XMLArchive* ar = AsXMLArchive())
			ar->EnterAttribute(name);
	}

	bool XMLArchiveAdapter::TryEnterAttribute(const String& name)
	{
		if (XMLArchive* ar = AsXMLArchive())
			return ar->TryEnterAttribute(name);
		return false;
	}

	void XMLArchiveAdapter::ExitAttribute()
	{
		if (XMLArchive* ar = AsXMLArchive())
			ar->ExitAttribute();
	}

	XMLArchive* XMLArchiveAdapter::AsXMLArchive()
	{
		return dynamic_cast<XMLArchive*>(m_Archive);
	}
}
