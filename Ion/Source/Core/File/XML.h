#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreTypes.h"
#include "Core/CoreMacros.h"

#include "rapidxml/rapidxml.hpp"

namespace Ion
{
	using XMLNode      = rapidxml::xml_node<char>;
	using XMLAttribute = rapidxml::xml_attribute<char>;

	class ION_API XMLDocument
	{
	public:
		XMLDocument(const String& xml);
		/* Takes the ownership of the xml character buffer */
		XMLDocument(char* xml);
		XMLDocument(File* xmlFile);
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

	protected:
		void InitXML(char* xml);

	protected:
		rapidxml::xml_document<char> m_XML;

	private:
		char* m_XMLString;
	};
}
