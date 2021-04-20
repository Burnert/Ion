#include "IonPCH.h"

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

	XMLDocument::XMLDocument(File* xmlFile)
	{
		ionassert(xmlFile->Exists());

		xmlFile->Open(IO::FM_Read);

		llong size = xmlFile->GetSize();
		ionassert(size > 0, "XML file cannot be empty!");

		char* xmlBuffer = new char[size + 1];
		memset(xmlBuffer, 0, size + 1);
		xmlFile->Read(xmlBuffer, size + 1);

		xmlFile->Close();

		InitXML(xmlBuffer);
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
