#include "Core/CorePCH.h"

#include "Core/Error/Error.h"
#include "Core/String/StringUtils.h"

#include "XMLArchive.h"

#include "rapidxml/rapidxml_print.hpp"

namespace Ion
{
	void XMLArchive::Serialize(void* const bytes, size_t size)
	{
		// @TODO: Encode the binary somehow (base64?)
	}

	void XMLArchive::Serialize(bool& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(int8& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(int16& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(int32& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(int64& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(uint8& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(uint16& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(uint32& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(uint64& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(float& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(double& value)
	{
		SerializeFundamental(value);
	}

	void XMLArchive::Serialize(String& value)
	{
		using XMLBase = rapidxml::xml_base<char>;

		XMLBase* element = m_CurrentNode ?
			(m_CurrentAttribute ? (XMLBase*)m_CurrentAttribute : (XMLBase*)m_CurrentNode) :
			nullptr;

		if (IsSaving())
		{
			char* str = m_XML->XML().allocate_string(value.c_str());
			element->value(str);
		}
		else if (IsLoading())
		{
			value = element->value();
		}
	}

	void XMLArchive::EnterNode(const String& name)
	{
		ionassert(!name.empty());

		if (IsLoading())
		{
			if (XMLNode* node = m_CurrentNode->first_node(name.c_str()))
				m_CurrentNode = node;
			else ionerror(IOError, "Node \"{}\" not found.", name);
		}
		else if (IsSaving())
		{
			XMLNode* node = m_XML->XML().allocate_node(rapidxml::node_element, m_XML->XML().allocate_string(name.c_str()));
			m_CurrentNode->append_node(node);
			m_CurrentNode = node;
		}
	}

	bool XMLArchive::TryEnterNode(const String& name)
	{
		ionassert(!name.empty());

		if (IsLoading())
		{
			XMLNode* node = m_CurrentNode->first_node(name.c_str());
			if (node)
				m_CurrentNode = node;
			return node;
		}
		else if (IsSaving())
		{
			XMLNode* node = m_XML->XML().allocate_node(rapidxml::node_element, m_XML->XML().allocate_string(name.c_str()));
			m_CurrentNode->append_node(node);
			m_CurrentNode = node;
			return true;
		}
		return false;
	}

	bool XMLArchive::TryEnterSiblingNode()
	{
		if (IsLoading())
		{
			XMLNode* node = m_CurrentNode->next_sibling(m_CurrentNode->name());
			if (node)
				m_CurrentNode = node;
			return node;
		}
		else if (IsSaving())
		{
			ionassert(m_CurrentNode->parent());
			XMLNode* node = m_XML->XML().allocate_node(rapidxml::node_element, m_XML->XML().allocate_string(m_CurrentNode->name()));
			m_CurrentNode->parent()->append_node(node);
			m_CurrentNode = node;
			return true;
		}
		return false;
	}

	void XMLArchive::ExitNode()
	{
		m_CurrentNode = m_CurrentNode->parent();
	}

	void XMLArchive::EnterAttribute(const String& name)
	{
		ionassert(!name.empty());
		ionassert(!m_CurrentAttribute);

		if (IsLoading())
		{
			if (XMLAttribute* attr = m_CurrentNode->first_attribute(name.c_str()))
				m_CurrentAttribute = attr;
			else ionerror(IOError, "Attribute \"{}\" not found.", name);
		}
		else if (IsSaving())
		{
			XMLAttribute* attr = m_XML->XML().allocate_attribute(m_XML->XML().allocate_string(name.c_str()));
			m_CurrentNode->append_attribute(attr);
			m_CurrentAttribute = attr;
		}
	}

	bool XMLArchive::TryEnterAttribute(const String& name)
	{
		ionassert(!name.empty());
		ionassert(!m_CurrentAttribute);

		if (IsLoading())
		{
			XMLAttribute* attr = m_CurrentNode->first_attribute(name.c_str());
			if (attr)
				m_CurrentAttribute = attr;
			return attr;
		}
		else if (IsSaving())
		{
			XMLAttribute* attr = m_XML->XML().allocate_attribute(m_XML->XML().allocate_string(name.c_str()));
			m_CurrentNode->append_attribute(attr);
			m_CurrentAttribute = attr;
			return true;
		}
		return false;
	}

	void XMLArchive::ExitAttribute()
	{
		m_CurrentAttribute = nullptr;
	}

	void XMLArchive::SeekRoot()
	{
		m_CurrentNode = &m_XML->XML();
		m_CurrentAttribute = nullptr;
	}

	void XMLArchive::LoadFromFile(File& file)
	{
		ionassert(IsLoading());

		if (file.Open(EFileMode::Read)
			.Err([&](Error& err) { SerializationLogger.Error("Cannot load XMLArchive from file \"{}\".\n{}", file.GetFullPath(), err.Message); }))
		{
			String sXML = file.Read()
				.Err([&](Error& err) { SerializationLogger.Error("Cannot read file \"{}\".\n{}", file.GetFullPath(), err.Message); })
				.UnwrapOr(EmptyString);

			if (!sXML.empty())
				LoadXML(std::make_shared<XMLDocument>(sXML));
		}
	}

	void XMLArchive::SaveToFile(File& file) const
	{
		ionassert(IsSaving());
		ionassert(m_XML);

		if (file.Open(EFileMode::Write | EFileMode::CreateNew | EFileMode::Reset)
			.Err([&](Error& err) { SerializationLogger.Error("Cannot save XMLArchive to file \"{}\".\n{}", file.GetFullPath(), err.Message); }))
		{
			std::stringstream ss;
			ss << SaveXML()->XML();

			if (file.Write(ss.str())
				.Err([&](Error& err) { SerializationLogger.Error("Cannot write to file \"{}\".\n{}", file.GetFullPath(), err.Message); }))
			{
			}
		}
	}

	void XMLArchive::LoadXML(const std::shared_ptr<XMLDocument>& xml)
	{
		ionassert(IsLoading());

		m_XML = xml;
		SeekRoot();
	}

	std::shared_ptr<XMLDocument> XMLArchive::SaveXML() const
	{
		ionassert(IsSaving());

		return m_XML;
	}
}
