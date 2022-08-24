#include "Core/CorePCH.h"

#include "XML.h"

namespace Ion
{
	XMLDocument::XMLDocument(const String& xml)
	{
		ionassert(xml.size() > 0, "XML String cannot be empty!");

		char* xmlBuffer = new char[xml.size() + 1];
		strcpy_s(xmlBuffer, xml.size() + 1, xml.c_str());

		InitXML(xmlBuffer);
	}

	XMLDocument::XMLDocument(char* xml)
	{
		ionassert(strlen(xml) > 0, "XML String cannot be empty!");

		InitXML(xml);
	}

	XMLDocument::~XMLDocument()
	{
		delete[] m_XMLString;
	}

	void XMLDocument::InitXML(char* xml)
	{
		TRACE_FUNCTION();

		m_XMLString = xml;

		m_XML.parse<0>(m_XMLString);
	}
}
