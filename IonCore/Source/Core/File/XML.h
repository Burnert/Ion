#pragma once

#include "Core/Base.h"
#include "Core/Error/Error.h"
#include "Core/Diagnostics/Tracing.h"

#include "rapidxml/rapidxml.hpp"

namespace Ion
{
	using XMLNode      = rapidxml::xml_node<char>;
	using XMLAttribute = rapidxml::xml_attribute<char>;

	class ION_API XMLDocument
	{
	public:
		// Create an empty XML document
		XMLDocument();
		XMLDocument(const String& xml);
		/* Takes the ownership of the xml character buffer */
		XMLDocument(char* xml);
		~XMLDocument();

		template<typename Pred>
		inline static XMLNode* FindNode(XMLNode* parentNode, const char* name, Pred predicate)
		{
			TRACE_FUNCTION();

			if (parentNode == nullptr || parentNode->first_node() == nullptr)
			{
				return nullptr;
			}
			XMLNode* nextNode = parentNode->first_node(name);
			ionassert(nextNode);
			do
			{
				if (predicate(nextNode))
				{
					return nextNode;
				}
			}
			while (nextNode = nextNode->next_sibling());
			return nullptr;
		}

		template<typename Pred>
		inline static XMLAttribute* FindAttribute(XMLNode* parentNode, const char* name, Pred predicate)
		{
			TRACE_FUNCTION();

			if (parentNode == nullptr || parentNode->first_attribute() == nullptr)
			{
				return nullptr;
			}
			XMLAttribute* nextAttribute = parentNode->first_attribute(name);
			ionassert(nextAttribute);
			do
			{
				if (predicate(nextAttribute))
				{
					return nextAttribute;
				}
			}
			while (nextAttribute = nextAttribute->next_attribute());
			return nullptr;
		}

		/* Returns a predicate that can be used in FindNode function */
		inline static auto WithAttributeValuePredicate(const char* name, const char* value)
		{
			return [=](XMLNode* node) {
				ionassert(node);
				return FindAttribute(node, name, [=](XMLAttribute* attribute) {
					ionassert(attribute);
					return _strcmpi(attribute->value(), value) == 0;
				});
			};
		}

		inline rapidxml::xml_document<char>& XML()
		{
			return m_XML;
		}

	protected:
		void InitXML(const String& xml);

	protected:
		rapidxml::xml_document<char> m_XML;

	private:
		String m_XMLString;
	};
}
