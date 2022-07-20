#include "IonPCH.h"

#include "XMLParser.h"

namespace Ion
{
	XMLParser::XMLParser(const FilePath& file) :
		m_Path(file),
		m_bFailed(false),
		m_CurrentNode(nullptr)
	{
	}

	XMLParser& XMLParser::Open()
	{
		ionassert(!IsOpen(), "Cannot open the file while it's already open.");
		ionassert(m_Path.IsFile());

		String xml;
		File::ReadToString(m_Path, xml);
		if (xml.empty())
		{
			Fail("The file is empty.");
			return *this;
		}

		m_XML = MakeShared<XMLDocument>(xml);
		m_CurrentNode = &m_XML->XML();

		return *this;
	}

	XMLParser& XMLParser::EnterNode(const String& nodeName)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());
		m_CurrentNode = node;

		return *this;
	}

	XMLParser& XMLParser::ExitNode()
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());
		ionassert(m_CurrentNode->parent());

		m_CurrentNode = m_CurrentNode->parent();

		return *this;
	}

	String XMLParser::GetCurrentNodeName() const
	{
		ionassert(IsOpen());

		return m_CurrentNode->name();
	}

	XMLParser& XMLParser::GetCurrentNodeName(String& outName)
	{
		ionassert(IsOpen());

		outName = m_CurrentNode->name();

		return *this;
	}

	XMLParser& XMLParser::ExpectNode(const String& nodeName)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		return *this;
	}

	XMLParserResult XMLParser::Finalize()
	{
		ionassert(IsOpen());

		if (m_ParseResult.OverallResult == EXMLParserResultType::Success)
		{
			AddMessage(EXMLParserResultType::Success, "Asset has been parsed successfully.");
		}

		m_XML.reset();
		m_CurrentNode = nullptr;

		return m_ParseResult;
	}

	void XMLParser::Fail(const String& message)
	{
		ionassert(IsOpen());

		m_bFailed = true;
		m_ParseResult.OverallResult = EXMLParserResultType::Fail;
		AddMessage(EXMLParserResultType::Fail, message);
	}

	void XMLParser::AddMessage(EXMLParserResultType type, const String& message)
	{
		ionassert(type != EXMLParserResultType::Fail || m_bFailed, "Call the Fail function directly.");
		m_ParseResult.Messages.emplace_back(XMLParserMessage { type, message });
	}

	const FilePath& XMLParser::GetPath() const
	{
		return m_Path;
	}
}
