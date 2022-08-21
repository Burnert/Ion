#pragma once

#include "Core/File/XML.h"

namespace Ion
{
	REGISTER_LOGGER(XMLParserLogger, "Core::File::XMLParser", ELoggerFlags::None, ELogLevel::Warn);

#define _PARSER_NODE_ERROR_MSG_PATTERN \
"<{}> node has not been found.\nIn file: \"{}\"\n"

#define _PARSER_ATTR_ERROR_MSG_PATTERN \
"{} attribute could not be found in node <{}>.\nIn file: \"{}\"\n"

	template<typename T>
	static const char* _ParserCheckNodeHelper(T str) { return nullptr; }
	template<>
	static const char* _ParserCheckNodeHelper(char* str) { return str; }
	template<>
	static const char* _ParserCheckNodeHelper(const char* str) { return str; }
	template<>
	static const char* _ParserCheckNodeHelper(const String& str) { return str.c_str(); }

#define _PARSER_CHECK_NODE_R(node, nodeName, path, ret) \
if (!(node)) \
{ \
	String sPath = path.ToString(); \
	String sMsg = fmt::format(_PARSER_NODE_ERROR_MSG_PATTERN, nodeName, sPath); \
	XMLParserLogger.Error("XMLParser error:\n{0}", sMsg); \
	Fail(sMsg); \
	return ret; \
}

#define _PARSER_CHECK_NODE(node, nodeName, path) _PARSER_CHECK_NODE_R(node, nodeName, path, *this)
#define _PARSER_CHECK_NODE_V(node, nodeName, path) _PARSER_CHECK_NODE_R(node, nodeName, path, )

#define _PARSER_CHECK_ATTR_R(attr, attrName, nodeName, path, ret) \
if (!(attr)) \
{ \
	String sPath = path.ToString(); \
	String sMsg = fmt::format(_PARSER_ATTR_ERROR_MSG_PATTERN, attrName, nodeName, sPath); \
	XMLParserLogger.Error("XMLParser error:\n{0}", sMsg); \
	Fail(sMsg); \
	return ret; \
}

#define _PARSER_CHECK_ATTR(attr, attrName, nodeName, path) _PARSER_CHECK_ATTR_R(attr, attrName, nodeName, path, *this)
#define _PARSER_CHECK_ATTR_V(attr, attrName, nodeName, path) _PARSER_CHECK_ATTR_R(attr, attrName, nodeName, path, )

#define _PARSER_FAILED_CHECK_R(ret) if (m_bFailed) return ret
#define _PARSER_FAILED_CHECK() _PARSER_FAILED_CHECK_R(*this)

	enum class EXMLParserResultType
	{
		Success = 0,
		Warning,
		Error,
		Fail
	};

	template<>
	struct TEnumParser<EXMLParserResultType>
	{
		ENUM_PARSER_TO_STRING_BEGIN(EXMLParserResultType)
		ENUM_PARSER_TO_STRING_HELPER(Success)
		ENUM_PARSER_TO_STRING_HELPER(Warning)
		ENUM_PARSER_TO_STRING_HELPER(Error)
		ENUM_PARSER_TO_STRING_HELPER(Fail)
		ENUM_PARSER_TO_STRING_END()

		ENUM_PARSER_FROM_STRING_BEGIN(EXMLParserResultType)
		ENUM_PARSER_FROM_STRING_HELPER(Success)
		ENUM_PARSER_FROM_STRING_HELPER(Warning)
		ENUM_PARSER_FROM_STRING_HELPER(Error)
		ENUM_PARSER_FROM_STRING_HELPER(Fail)
		ENUM_PARSER_FROM_STRING_END()
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

		inline bool OK() const
		{
#if ION_DEBUG // Always print XMLParser errors on Debug configuration
			if (OverallResult == EXMLParserResultType::Fail)
			{
				XMLParserLogger.Error("XMLParser failed messages:");

				for (const XMLParserMessage& message : Messages)
				{
					ELogLevel level = [&]
					{
						switch (message.Type)
						{
						case EXMLParserResultType::Success: return ELogLevel::Info;
						case EXMLParserResultType::Warning: return ELogLevel::Warn;
						case EXMLParserResultType::Error:   return ELogLevel::Error;
						case EXMLParserResultType::Fail:    return ELogLevel::Critical;
						default:                            return ELogLevel::Trace;
						}
					}();
					
					XMLParserLogger.Log(level, "{}: {}", TEnumParser<EXMLParserResultType>::ToString(message.Type), message.Text);
				}
			}
#endif
			return OverallResult != EXMLParserResultType::Fail;
		}
	};

	template<typename T>
	class ION_API XMLParser
	{
	public:
		using TThis = XMLParser<T>;
		using TFinalClass = TIf<TIsSameV<T, void>, TThis, T>;

		// Message interface
		struct MessageInterface
		{
			MessageInterface(TThis* owner);

			void SendWarning(const String& text) const;
			void SendError(const String& text) const;
			void SendFail(const String& text) const;

		private:
			TThis* m_Owner;
		};

		using FParseFunc = TFunction<void(String)>;
		using FParseFuncEx = TFunction<void(const MessageInterface&, String)>;

		template<typename TAttr>
		using TFParseFunc = TFunction<void(TAttr)>;

		using FExpectFunc = TFunction<bool(String)>;

		using FEnterEachNodeFunc = TFunction<void(TFinalClass&)>;
		using FEnterFunc = TFunction<void(TFinalClass&)>;

		XMLParser(const FilePath& file);

		TFinalClass& Open();

		TFinalClass& EnterNode(const String& nodeName);
		TFinalClass& ExitNode();

		template<typename FEnter>
		TFinalClass& TryEnterNode(const String& nodeName, FEnter onEnter);

		// Current node functions --------------------------------------------------------

		TFinalClass& GetCurrentAttribute(const String& attrName, String& outString);

		template<typename FParse>
		TFinalClass& ParseCurrentNodeValue(FParse parseFunc);
		template<typename... Args>
		TFinalClass& ParseCurrentAttributes(Args&&... args);

		template<typename TEnum>
		TFinalClass& ParseCurrentEnumAttribute(const String& attrName, TEnum& outEnum);
		template<typename TEnum, typename FParse>
		TFinalClass& ParseCurrentEnumAttribute(const String& attrName, FParse parseFunc);

		template<typename TAttr>
		TFinalClass& ParseCurrentAttributeTyped(const String& attrName, TAttr& outValue);
		template<typename TAttr, typename FParse>
		TFinalClass& ParseCurrentAttributeTyped(const String& attrName, FParse parseFunc);

		template<typename... Args>
		TFinalClass& TryParseCurrentAttributes(Args&&... args);

		template<typename FExpect>
		TFinalClass& ExpectCurrentNodeValue(FExpect expectFunc);
		template<typename... Args>
		TFinalClass& ExpectCurrentAttributes(Args&&... args);

		String GetCurrentNodeName() const;
		TFinalClass& GetCurrentNodeName(String& outName);

		// Arbitrary node functions --------------------------------------------------------

		template<typename FParse>
		TFinalClass& ParseNodeValue(const String& nodeName, FParse parseFunc);
		template<typename... Args>
		TFinalClass& ParseAttributes(const String& nodeName, Args&&... args);
		template<typename... Args>
		TFinalClass& ParseAttributesAndEnterNode(const String& nodeName, Args&&... args);

		template<typename FParse>
		TFinalClass& TryParseNodeValue(const String& nodeName, FParse parseFunc);
		template<typename... Args>
		TFinalClass& TryParseAttributes(const String& nodeName, Args&&... args);

		bool CheckNode(const String& nodeName) const;

		TFinalClass& ExpectNode(const String& nodeName);

		template<typename FExpect>
		TFinalClass& ExpectNodeValue(const String& nodeName, FExpect expectFunc);
		template<typename... Args>
		TFinalClass& ExpectAttributes(const String& nodeName, Args&&... args);

		template<typename FForEach>
		TFinalClass& EnterEachNode(FForEach forEach);
		template<typename FForEach>
		TFinalClass& EnterEachNode(const String& nodeName, FForEach forEach);

		// Other functions -------------------------------------------------------------------

		template<typename FThen>
		TFinalClass& If(bool bCondition, FThen then);

		void Fail(const String& message);

		template<typename FPred>
		TFinalClass& FailIf(FPred pred, const String& message);

		void AddMessage(EXMLParserResultType type, const String& message);

		XMLParserResult Finalize();

		MessageInterface GetInterface();

		bool IsOpen() const;

		const FilePath& GetPath() const;

	private:
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

		inline operator TFinalClass&() const
		{
			return *(TFinalClass*)this;
		}

	private:
		FilePath m_Path;

		std::shared_ptr<XMLDocument> m_XML;
		XMLNode* m_CurrentNode;

		XMLParserResult m_ParseResult;

		bool m_bFailed;

		friend struct TThis::MessageInterface;
	};

	using GenericXMLParser = XMLParser<void>;

	// Message Interface impl --------------------------------------------------------

	template<typename T>
	inline XMLParser<T>::MessageInterface::MessageInterface(TThis* owner) :
		m_Owner(owner)
	{
	}

	template<typename T>
	inline void XMLParser<T>::MessageInterface::SendWarning(const String& text) const
	{
		m_Owner->AddMessage(EXMLParserResultType::Warning, text);
	}

	template<typename T>
	inline void XMLParser<T>::MessageInterface::SendError(const String& text) const
	{
		m_Owner->AddMessage(EXMLParserResultType::Error, text);
	}

	template<typename T>
	inline void XMLParser<T>::MessageInterface::SendFail(const String& text) const
	{
		m_Owner->Fail(text);
	}

	// XMLParser ---------------------------------------------------------------------------

	template<typename T>
	XMLParser<T>::XMLParser(const FilePath& file) :
		m_Path(file),
		m_bFailed(false),
		m_CurrentNode(nullptr)
	{
		ionassert(!file.IsEmpty());
		ionassert(file.IsFile());
	}

	template<typename T>
	typename XMLParser<T>::TFinalClass& XMLParser<T>::Open()
	{
		ionassert(!IsOpen(), "Cannot open the file while it's already open.");
		ionassert(m_Path.IsFile());

		String xml;
		ionmatchresult(File::ReadToString(m_Path),
			mcaseok xml = R.Unwrap();
			melse
			{
				Fail("Cannot read the file.");
				return *this;
			}
		);
		
		if (xml.empty())
		{
			Fail("The file is empty.");
			return *this;
		}

		m_XML = std::make_shared<XMLDocument>(xml);
		m_CurrentNode = &m_XML->XML();

		return *this;
	}

	template<typename T>
	typename XMLParser<T>::TFinalClass& XMLParser<T>::EnterNode(const String& nodeName)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());
		m_CurrentNode = node;

		return *this;
	}

	template<typename T>
	typename XMLParser<T>::TFinalClass& XMLParser<T>::ExitNode()
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());
		ionassert(m_CurrentNode->parent());

		m_CurrentNode = m_CurrentNode->parent();

		return *this;
	}

	template<typename T>
	template<typename FEnter>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::TryEnterNode(const String& nodeName, FEnter onEnter)
	{
		static_assert(TIsConvertibleV<FEnter, FEnterFunc>);

		if (CheckNode(nodeName))
		{
			EnterNode(nodeName);
			onEnter(*this);
			ExitNode();
		}

		return *this;
	}

	// Current node functions --------------------------------------------------------

	template<typename T>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::GetCurrentAttribute(const String& attrName, String& outString)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLAttribute* attr = m_CurrentNode->first_attribute(attrName.c_str());
		if (attr)
			outString = attr->value();

		return *this;
	}

	template<typename T>
	template<typename FParse>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseCurrentNodeValue(FParse parseFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseNodeValue(m_CurrentNode, parseFunc);

		return *this;
	}

	template<typename T>
	template<typename ...Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseCurrentAttributes(Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseAttributesExpand<false>(m_CurrentNode, args...);

		return *this;
	}

	template<typename T>
	template<typename TEnum>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseCurrentEnumAttribute(const String& attrName, TEnum& outEnum)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseCurrentAttributes(
			attrName, [this, &attrName, &outEnum](String sValue)
			{
				TOptional<TEnum> attrValue = TEnumParser<TEnum>::FromString(sValue);
				if (attrValue)
				{
					outEnum = *attrValue;
				}
				else
				{
					String message = fmt::format("Cannot parse enum attribute value: <{0} {1}=\"{2}\"> -> T", GetCurrentNodeName(), attrName, sValue);
					AddMessage(EXMLParserResultType::Error, message);
				}
			});

		return *this;
	}

	template<typename T>
	template<typename TEnum, typename FParse>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseCurrentEnumAttribute(const String& attrName, FParse parseFunc)
	{
		static_assert(TIsConvertibleV<FParse, TFParseFunc<TEnum>>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseCurrentAttributes(
			attrName, [this, &attrName &parseFunc](String sValue)
			{
				TOptional<TEnum> attrValue = TEnumParser<TEnum>::FromString(sValue);
				if (attrValue)
				{
					parseFunc(*attrValue);
				}
				else
				{
					String message = fmt::format("Cannot parse enum attribute value: <{0} {1}=\"{2}\"> -> T", GetCurrentNodeName(), attrName, sValue);
					AddMessage(EXMLParserResultType::Error, message);
				}
			});

		return *this;
	}

	template<typename T>
	template<typename TAttr>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseCurrentAttributeTyped(const String& attrName, TAttr& outValue)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseCurrentAttributes(
			attrName, [this, &attrName, &outValue](String sValue)
			{
				TOptional<TAttr> attrValue = TStringParser<TAttr>()(sValue);
				if (attrValue)
				{
					outValue = *attrValue;
				}
				else
				{
					String message = fmt::format("Cannot parse attribute value: <{0} {1}=\"{2}\"> -> T", GetCurrentNodeName(), attrName, sValue);
					AddMessage(EXMLParserResultType::Error, message);
				}
			});

		return *this;
	}

	template<typename T>
	template<typename TAttr, typename FParse>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseCurrentAttributeTyped(const String& attrName, FParse parseFunc)
	{
		static_assert(TIsConvertibleV<FParse, TFParseFunc<TAttr>>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseCurrentAttributes(
			attrName, [this, &attrName, &parseFunc](String sValue)
			{
				TOptional<TAttr> attrValue = TStringParser<TAttr>()(sValue);
				if (attrValue)
				{
					parseFunc(*attrValue);
				}
				else
				{
					// @TODO: type names
					String message = fmt::format("Cannot parse attribute value: <{0} {1}=\"{2}\"> -> T", GetCurrentNodeName(), attrName, sValue);
					AddMessage(EXMLParserResultType::Error, message);
				}
			});

		return *this;
	}

	template<typename T>
	template<typename ...Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::TryParseCurrentAttributes(Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ParseAttributesExpand<true>(m_CurrentNode, args...);

		return *this;
	}

	template<typename T>
	template<typename FExpect>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ExpectCurrentNodeValue(FExpect expectFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ExpectNodeValue(m_CurrentNode, expectFunc);
	}

	template<typename T>
	template<typename ...Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ExpectCurrentAttributes(Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		ExpectAttributesExpand(m_CurrentNode, args...);
	}

	template<typename T>
	String XMLParser<T>::GetCurrentNodeName() const
	{
		ionassert(IsOpen());

		return m_CurrentNode->name();
	}

	template<typename T>
	typename XMLParser<T>::TFinalClass& XMLParser<T>::GetCurrentNodeName(String& outName)
	{
		ionassert(IsOpen());

		outName = m_CurrentNode->name();

		return *this;
	}

	// Arbitrary node functions --------------------------------------------------------

	template<typename T>
	template<typename FParse>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseNodeValue(const String& nodeName, FParse parseFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ParseNodeValue(node, parseFunc);

		return *this;
	}

	template<typename T>
	template<typename... Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseAttributes(const String& nodeName, Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ParseAttributesExpand<false>(node, args...);

		return *this;
	}

	template<typename T>
	template<typename... Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ParseAttributesAndEnterNode(const String& nodeName, Args&&... args)
	{
		ParseAttributes(nodeName, Forward<Args>(args)...);
		return EnterNode(nodeName);
	}

	template<typename T>
	template<typename FParse>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::TryParseNodeValue(const String& nodeName, FParse parseFunc)
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

	template<typename T>
	template<typename ...Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::TryParseAttributes(const String& nodeName, Args&&... args)
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

	template<typename T>
	bool XMLParser<T>::CheckNode(const String& nodeName) const
	{
		_PARSER_FAILED_CHECK_R(false);
		ionassert(IsOpen());

		return (bool)m_CurrentNode->first_node(nodeName.c_str());
	}

	template<typename T>
	typename XMLParser<T>::TFinalClass& XMLParser<T>::ExpectNode(const String& nodeName)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		return *this;
	}

	template<typename T>
	template<typename FExpect>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ExpectNodeValue(const String& nodeName, FExpect expectFunc)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());
		ionassert(m_CurrentNode);

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ExpectNodeValue(node, expectFunc);

		return *this;
	}

	template<typename T>
	template<typename ...Args>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::ExpectAttributes(const String& nodeName, Args&&... args)
	{
		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* node = m_CurrentNode->first_node(nodeName.c_str());
		_PARSER_CHECK_NODE(node, nodeName, GetPath());

		ExpectAttributesExpand(node, args...);

		return *this;
	}

	template<typename T>
	template<typename FForEach>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::EnterEachNode(FForEach forEach)
	{
		static_assert(TIsConvertibleV<FForEach, FEnterEachNodeFunc>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* previousNode = m_CurrentNode;
		for (m_CurrentNode = m_CurrentNode->first_node(); m_CurrentNode; m_CurrentNode = m_CurrentNode->next_sibling())
		{
			forEach(*(TFinalClass*)this);
		}
		m_CurrentNode = previousNode;

		return *this;
	}

	template<typename T>
	template<typename FForEach>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::EnterEachNode(const String& nodeName, FForEach forEach)
	{
		static_assert(TIsConvertibleV<FForEach, FEnterEachNodeFunc>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		XMLNode* previousNode = m_CurrentNode;
		for (m_CurrentNode = m_CurrentNode->first_node(nodeName.c_str()); m_CurrentNode; m_CurrentNode = m_CurrentNode->next_sibling(nodeName.c_str()))
		{
			forEach(*(TFinalClass*)this);
		}
		m_CurrentNode = previousNode;

		return *this;
	}

	template<typename T>
	template<typename FThen>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::If(bool bCondition, FThen then)
	{
		static_assert(TIsConvertibleV<FThen, TFunction<void(TFinalClass&)>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		if (bCondition)
			then(*this);

		return *this;
	}

	template<typename T>
	template<typename FPred>
	inline typename XMLParser<T>::TFinalClass& XMLParser<T>::FailIf(FPred pred, const String& message)
	{
		static_assert(TIsConvertibleV<FPred, TFunction<bool()>>);

		_PARSER_FAILED_CHECK();
		ionassert(IsOpen());

		if (pred())
			Fail(message);

		return *this;
	}

	template<typename T>
	XMLParserResult XMLParser<T>::Finalize()
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

	template<typename T>
	inline typename XMLParser<T>::MessageInterface XMLParser<T>::GetInterface()
	{
		ionassert(IsOpen());

		return MessageInterface(this);
	}

	template<typename T>
	inline bool XMLParser<T>::IsOpen() const
	{
		return (bool)m_XML;
	}

	template<typename T>
	void XMLParser<T>::Fail(const String& message)
	{
		ionassert(IsOpen());

		m_bFailed = true;
		m_ParseResult.OverallResult = EXMLParserResultType::Fail;
		//AddMessage(EXMLParserResultType::Fail, message);
		m_ParseResult.Messages.emplace_back(XMLParserMessage { EXMLParserResultType::Fail, message });

		XMLParserLogger.Error("XMLParser could not parse file \"{}\". Check the messages for more info.", m_Path.ToString());
	}

	template<typename T>
	void XMLParser<T>::AddMessage(EXMLParserResultType type, const String& message)
	{
		ionassert(type != EXMLParserResultType::Fail, "Call the Fail function directly.");
		m_ParseResult.Messages.emplace_back(XMLParserMessage { type, message });
	}

	template<typename T>
	const FilePath& XMLParser<T>::GetPath() const
	{
		return m_Path;
	}

	// Implementation detail -------------------------------------------------------------------------------

	template<typename T>
	template<typename FParse>
	inline void XMLParser<T>::ParseNodeValue(XMLNode* node, FParse parseFunc)
	{
		static_assert(
			TIsConvertibleV<FParse, FParseFunc> ||
			TIsConvertibleV<FParse, FParseFuncEx>);

		if constexpr (TIsConvertibleV<FParse, FParseFuncEx>)
		{
			parseFunc(XMLParser<T>::MessageInterface(this), node->value());
		}
		else
		{
			parseFunc(node->value());
		}
	}

	template<typename T>
	template<bool bTry, typename FParse, typename ...Args>
	inline void XMLParser<T>::ParseAttributesExpand(XMLNode* node, const String& name, FParse parseFunc, Args&&... rest)
	{
		ParseAttribute<bTry>(node, name, parseFunc);

		if (m_bFailed)
			return;

		if constexpr (sizeof...(Args) > 0)
		{
			ParseAttributesExpand<bTry>(node, rest...);
		}
	}

	template<typename T>
	template<bool bTry, typename FParse>
	inline void XMLParser<T>::ParseAttribute(XMLNode* node, const String& name, FParse parseFunc)
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
			parseFunc(XMLParser<T>::MessageInterface(this), attribute->value());
		}
		else
		{
			parseFunc(attribute->value());
		}
	}

	template<typename T>
	template<typename FExpect>
	inline void XMLParser<T>::ExpectNodeValue(XMLNode* node, FExpect expectFunc)
	{
		if (!expectFunc(m_CurrentNode->value()))
		{
			String message = fmt::format("Unexpected node value: <{0}> -> \"{1}\"", m_CurrentNode->name(), m_CurrentNode->value());
			Fail(message);
		}
	}

	template<typename T>
	template<typename FExpect, typename ...Args>
	inline void XMLParser<T>::ExpectAttributesExpand(XMLNode* node, const String& name, FExpect expectFunc, Args&&... rest)
	{
		ExpectAttribute(node, name, expectFunc);

		if (m_bFailed)
			return;

		if constexpr (sizeof...(Args) > 0)
		{
			ExpectAttributesExpand(node, rest...);
		}
	}

	template<typename T>
	template<typename FExpect>
	inline void XMLParser<T>::ExpectAttribute(XMLNode* node, const String& name, FExpect expectFunc)
	{
		static_assert(TIsConvertibleV<FExpect, FExpectFunc>);

		XMLAttribute* attribute = node->first_attribute(name.c_str());
		_PARSER_CHECK_ATTR_V(attribute, name, node->name(), GetPath());

		if (!expectFunc(attribute->value()))
		{
			String message = fmt::format("Unexpected attribute value: <{0} {1}=\"{2}\">", node->name(), name, attribute->value());
			Fail(message);
		}
	}
}
