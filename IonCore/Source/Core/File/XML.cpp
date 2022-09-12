#include "Core/CorePCH.h"

#include "XML.h"

namespace Ion
{
	XMLDocument::XMLDocument()
	{
	}

	XMLDocument::XMLDocument(const String& xml)
	{
		ionassert(xml.size() > 0, "XML String cannot be empty!");

		InitXML(xml);
	}

	XMLDocument::XMLDocument(char* xml)
	{
		ionassert(strlen(xml) > 0, "XML String cannot be empty!");

		InitXML(xml);
	}

	XMLDocument::~XMLDocument()
	{
	}

	void XMLDocument::InitXML(const String& xml)
	{
		TRACE_FUNCTION();

		m_XMLString = xml;

		m_XML.parse<0>(m_XMLString.data());
	}
}
