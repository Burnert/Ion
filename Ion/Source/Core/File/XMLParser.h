#pragma once

#include "Core/File/XML.h"

namespace Ion
{
#define _PARSER_NODE_EXCEPT_MESSAGE \
"<%s> node has not been found.\n"
#define _PARSER_NODE_ERROR_MESSAGE(nodeStr) \
nodeStr + " node has not been found.\n"

#define _PARSER_ATTR_EXCEPT_MESSAGE \
"%s attribute could not be found in node <%s>.\n"
#define _PARSER_ATTR_ERROR_MESSAGE(attrStr, nodeStr) \
attrStr + " attribute could not be found in node <" + nodeStr + ">.\n"

	template<typename T>
	static const char* _ParserCheckNodeHelper(T str) { return nullptr; }
	template<>
	static const char* _ParserCheckNodeHelper(char* str) { return str; }
	template<>
	static const char* _ParserCheckNodeHelper(const char* str) { return str; }
	template<>
	static const char* _ParserCheckNodeHelper(const String& str) { return str.c_str(); }

#define _PARSER_CHECK_NODE_R(node, nodeName, path, ret) \
ionexcept(node, _PARSER_NODE_EXCEPT_MESSAGE, \
	StringConverter::WStringToString(path.ToString()).c_str(), \
	_ParserCheckNodeHelper(nodeName)) \
	{ Fail(_PARSER_NODE_ERROR_MESSAGE(nodeName)); return ret; }

#define _PARSER_CHECK_NODE(node, nodeName, path) _PARSER_CHECK_NODE_R(node, nodeName, path, *this)
#define _PARSER_CHECK_NODE_V(node, nodeName, path) _PARSER_CHECK_NODE_R(node, nodeName, path, )

#define _PARSER_CHECK_ATTR_R(attr, attrName, nodeName, path, ret) \
ionexcept(attr, _PARSER_ATTR_EXCEPT_MESSAGE, \
	StringConverter::WStringToString(path.ToString()).c_str(), \
	_ParserCheckNodeHelper(attrName), _ParserCheckNodeHelper(nodeName)) \
	{ Fail(_PARSER_ATTR_ERROR_MESSAGE(attrName, nodeName)); return ret; }

#define _PARSER_CHECK_ATTR(attr, attrName, nodeName, path) _PARSER_CHECK_ATTR_R(attr, attrName, nodeName, path, *this)
#define _PARSER_CHECK_ATTR_V(attr, attrName, nodeName, path) _PARSER_CHECK_ATTR_R(attr, attrName, nodeName, path, )

#define _PARSER_FAILED_CHECK() if (m_bFailed) return *this

	enum class EXMLParserResultType
	{
		Success = 0,
		Warning,
		Error,
		Fail
	};

	struct XMLParserMessage
	{
		EXMLParserResultType Type = EXMLParserResultType::Success;
		String Text;
	};

	struct XMLParserResult
	{
		EXMLParserResultType OverallResult = EXMLParserResultType::Success;
		TArray<XMLParserMessage> Messages;
	};

	class ION_API XMLParser
	{
	public:
		// Message interface
		struct MessageInterface
		{
			MessageInterface(XMLParser* owner);

			void SendWarning(const String& text) const;
			void SendError(const String& text) const;
			void SendFail(const String& text) const;

		private:
			XMLParser* m_Owner;
		};

		using FParseFunc = TFunction<void(String)>;
		using FParseFuncEx = TFunction<void(const MessageInterface&, String)>;

		using FExpectFunc = TFunction<bool(String)>;

		using FEnterEachNodeFunc = TFunction<void(XMLParser&)>;

		XMLParser(const FilePath& file);

		XMLParser& Open();

		XMLParser& EnterNode(const String& nodeName);
		XMLParser& ExitNode();

		// Current node functions --------------------------------------------------------

		template<typename FParse>
		XMLParser& ParseCurrentNodeValue(FParse parseFunc);
		template<typename... Args>
		XMLParser& ParseCurrentAttributes(Args&&... args);

		template<typename... Args>
		XMLParser& TryParseCurrentAttributes(Args&&... args);

		template<typename FExpect>
		XMLParser& ExpectCurrentNodeValue(FExpect expectFunc);
		template<typename... Args>
		XMLParser& ExpectCurrentAttributes(Args&&... args);

		String GetCurrentNodeName() const;
		XMLParser& GetCurrentNodeName(String& outName);

		// Arbitrary node functions --------------------------------------------------------

		template<typename FParse>
		XMLParser& ParseNodeValue(const String& nodeName, FParse parseFunc);
		template<typename... Args>
		XMLParser& ParseAttributes(const String& nodeName, Args&&... args);
		template<typename... Args>
		XMLParser& ParseAttributesAndEnterNode(const String& nodeName, Args&&... args);

		template<typename FParse>
		XMLParser& TryParseNodeValue(const String& nodeName, FParse parseFunc);
		template<typename... Args>
		XMLParser& TryParseAttributes(const String& nodeName, Args&&... args);

		XMLParser& ExpectNode(const String& nodeName);

		template<typename FExpect>
		XMLParser& ExpectNodeValue(const String& nodeName, FExpect expectFunc);
		template<typename... Args>
		XMLParser& ExpectAttributes(const String& nodeName, Args&&... args);

		template<typename FForEach>
		XMLParser& EnterEachNode(const String& nodeName, FForEach forEach);

		XMLParserResult Finalize();

		MessageInterface GetInterface();

		bool IsOpen() const;

	private:
		void Fail(const String& message);

		void AddMessage(EXMLParserResultType type, const String& message);

		const FilePath& GetPath() const;

		template<typename FParse>
		void ParseNodeValue(XMLNode* node, FParse parseFunc);

		template<bool bTry, typename FParse, typename... Args>
		void ParseAttributesExpand(XMLNode* node, const String& name, FParse parseFunc, Args&&... rest);
		template<bool bTry, typename FParse>
		void ParseAttribute(XMLNode* node, const String& name, FParse parseFunc);

		template<typename FExpect>
		void ExpectNodeValue(XMLNode* node, FExpect expectFunc);

		template<typename FExpect, typename... Args>
		void ExpectAttributesExpand(XMLNode* node, const String& name, FExpect expectFunc, Args&&... rest);
		template<typename FExpect>
		void ExpectAttribute(XMLNode* node, const String& name, FExpect expectFunc);

	private:
		FilePath m_Path;

		TShared<XMLDocument> m_XML;
		XMLNode* m_CurrentNode;

		XMLParserResult m_ParseResult;

		bool m_bFailed;

		friend struct XMLParser::MessageInterface;
	};

	// Message Interface impl --------------------------------------------------------

	inline XMLParser::MessageInterface::MessageInterface(XMLParser* owner) :
		m_Owner(owner)
	{
	}

	inline void XMLParser::MessageInterface::SendWarning(const String& text) const
	{
		m_Owner->AddMessage(EXMLParserResultType::Warning, text);
	}

	inline void XMLParser::MessageInterface::SendError(const String& text) const
	{
		m_Owner->AddMessage(EXMLParserResultType::Error, text);
	}

	inline void XMLParser::MessageInterface::SendFail(const String& text) const
	{
		m_Owner->Fail(text);
	}

	// Current node functions --------------------------------------------------------

	template<typename FParse>
	inline XMLParser& XMLParser::ParseCurrentNodeValue(FParse parseFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseNodeValue(m_CurrentNode, parseFunc);

		return *this;
	}

	template<typename ...Args>
	inline XMLParser& XMLParser::ParseCurrentAttributes(Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseAttributesExpand<false>(m_CurrentNode, args...);

		return *this;
	}

	template<typename ...Args>
	inline XMLParser& XMLParser::TryParseCurrentAttributes(Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseAttributesExpand<true>(m_CurrentNode, args...);

		return *this;
	}

	template<typename FExpect>
	inline XMLParser& XMLParser::ExpectCurrentNodeValue(FExpect expectFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());
		ionassert(m_CurrentNode);

		ExpectNodeValue(m_CurrentNode, expectFunc);
	}

	template<typename ...Args>
	inline XMLParser& XMLParser::ExpectCurrentAttributes(Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ExpectAttributesExpand(m_CurrentNode, args...);
	}

	// Arbitrary node functions --------------------------------------------------------

	template<typename FParse>
	inline XMLParser& XMLParser::ParseNodeValue(const String& nodeName, FParse parseFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ParseNodeValue(node, parseFunc);

		return *this;
	}

	template<typename... Args>
	inline XMLParser& XMLParser::ParseAttributes(const String& nodeName, Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ParseAttributesExpand<false>(node, args...);

		return *this;
	}

	template<typename... Args>
	inline XMLParser& XMLParser::ParseAttributesAndEnterNode(const String& nodeName, Args&&... args)
	{
		ParseAttributes(nodeName, Forward<Args>(args)...);
		return EnterNode(nodeName);
	}

	template<typename FParse>
	inline XMLParser& XMLParser::TryParseNodeValue(const String& nodeName, FParse parseFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		if (node)
		{
			ParseNodeValue(node, parseFunc);
		}

		return *this;
	}

	template<typename ...Args>
	inline XMLParser& XMLParser::TryParseAttributes(const String& nodeName, Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		if (node)
		{
			ParseAttributesExpand<true>(node, args...);
		}

		return *this;
	}

	template<typename FExpect>
	inline XMLParser& XMLParser::ExpectNodeValue(const String& nodeName, FExpect expectFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());
		ionassert(m_CurrentNode);

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ExpectNodeValue(node, expectFunc);

		return *this;
	}

	template<typename ...Args>
	inline XMLParser& XMLParser::ExpectAttributes(const String& nodeName, Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ExpectAttributesExpand(node, args...);

		return *this;
	}

	template<typename FForEach>
	inline XMLParser& XMLParser::EnterEachNode(const String& nodeName, FForEach forEach)
	{
		static_assert(TIsConvertibleV<FForEach, FEnterEachNodeFunc>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* previousNode = m_CurrentNode;
		for (m_CurrentNode = m_CurrentNode->first_node(nodeName.c_str()); m_CurrentNode; m_CurrentNode = m_CurrentNode->next_sibling(nodeName.c_str()))
		{
			forEach(*this);
		}
		m_CurrentNode = previousNode;

		return *this;
	}

	inline XMLParser::MessageInterface XMLParser::GetInterface()
	{
		ionassert(IsOpen());

		return MessageInterface(this);
	}

	inline bool XMLParser::IsOpen() const
	{
		return (bool)m_XML;
	}

	// Implementation detail -------------------------------------------------------------------------------

	template<typename FParse>
	inline void XMLParser::ParseNodeValue(XMLNode* node, FParse parseFunc)
	{
		static_assert(
			TIsConvertibleV<FParse, FParseFunc> ||
			TIsConvertibleV<FParse, FParseFuncEx>);

		if constexpr (TIsConvertibleV<FParse, FParseFuncEx>)
		{
			parseFunc(XMLParser::MessageInterface(this), node->value());
		}
		else
		{
			parseFunc(node->value());
		}
	}

	template<bool bTry, typename FParse, typename ...Args>
	inline void XMLParser::ParseAttributesExpand(XMLNode* node, const String& name, FParse parseFunc, Args&&... rest)
	{
		ParseAttribute<bTry>(node, name, parseFunc);

		if (m_bFailed)
			return;

		if constexpr (sizeof...(Args) > 0)
		{
			ParseAttributesExpand<bTry>(node, rest...);
		}
	}

	template<bool bTry, typename FParse>
	inline void XMLParser::ParseAttribute(XMLNode* node, const String& name, FParse parseFunc)
	{
		static_assert(
			TIsConvertibleV<FParse, FParseFunc> ||
			TIsConvertibleV<FParse, FParseFuncEx>);

		XMLAttribute* attribute = node->first_attribute(name.c_str());
		if constexpr (!bTry)
		{
			_PARSER_CHECK_ATTR_V(attribute, name, node->name(), GetPath());
		}
		else if (!attribute)
		{
			return;
		}

		if constexpr (TIsConvertibleV<FParse, FParseFuncEx>)
		{
			parseFunc(XMLParser::MessageInterface(this), attribute->value());
		}
		else
		{
			parseFunc(attribute->value());
		}
	}

	template<typename FExpect>
	inline void XMLParser::ExpectNodeValue(XMLNode* node, FExpect expectFunc)
	{
		if (!expectFunc(m_CurrentNode->value()))
		{
			String message = fmt::format("Unexpected node value: <{0}> -> \"{1}\"", m_CurrentNode->name(), m_CurrentNode->value());
			Fail(message);
		}
	}

	template<typename FExpect, typename ...Args>
	inline void XMLParser::ExpectAttributesExpand(XMLNode* node, const String& name, FExpect expectFunc, Args&&... rest)
	{
		ExpectAttribute(node, name, expectFunc);

		if (m_bFailed)
			return;

		if constexpr (sizeof...(Args) > 0)
		{
			ExpectAttributesExpand(node, rest...);
		}
	}

	template<typename FExpect>
	inline void XMLParser::ExpectAttribute(XMLNode* node, const String& name, FExpect expectFunc)
	{
		static_assert(TIsConvertibleV<FExpect, FExpectFunc>);

		XMLAttribute* attribute = node->first_attribute(name.c_str());
		_PARSER_CHECK_ATTR_V(attribute, name, node->name(), GetPath());

		if (!expectFunc(attribute->value()))
		{
			String message = fmt::format("Unexpected attribute value: <{0}> -> ({1}=\"{2}\")", node->name(), name, attribute->value());
			Fail(message);
		}
	}
}
