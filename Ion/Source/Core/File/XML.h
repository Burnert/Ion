#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreTypes.h"
#include "Core/CoreMacros.h"

#include "rapidxml/rapidxml.hpp"

namespace Ion
{
	class ION_API XMLDocument
	{
	public:
		using XMLNode      = rapidxml::xml_node<char>;
		using XMLAttribute = rapidxml::xml_attribute<char>;

		XMLDocument(const String& xml);
		/* Takes the ownership of the xml character buffer */
		XMLDocument(char* xml);
		XMLDocument(File* xmlFile);
		~XMLDocument();

		template<typename Pred>
		inline XMLNode* FindNode(XMLNode* parentNode, const char* name, Pred predicate) const
		{
			XMLNode* nextNode = parentNode ? parentNode->first_node(name) : m_XML.first_node();
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
		inline XMLAttribute* FindAttribute(XMLNode* parentNode, const char* name, Pred predicate) const
		{
			if (parentNode == nullptr)
			{
				return nullptr;
			}
			XMLAttribute* nextAttribute = parentNode ? parentNode->first_attribute(name) : m_XML.first_attribute();
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
		inline auto WithAttributeValuePredicate(const char* name, const char* value)
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
