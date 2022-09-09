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
		if (XMLNode* node = m_CurrentNode->first_node(name.c_str()))
			m_CurrentNode = node;
		else ionerror(IOError);
	}

	bool XMLArchive::TryEnterNode(const String& name)
	{
		XMLNode* node = m_CurrentNode->first_node(name.c_str());
		if (node)
			m_CurrentNode = node;

		return node;
	}

	bool XMLArchive::TryEnterSiblingNode()
	{
		XMLNode* node = m_CurrentNode->next_sibling(m_CurrentNode->name());
		if (node)
			m_CurrentNode = node;

		return node;
	}

	void XMLArchive::ExitNode()
	{
		m_CurrentNode = m_CurrentNode->parent();
	}

	void XMLArchive::EnterAttribute(const String& name)
	{
		ionassert(!m_CurrentAttribute);
		if (XMLAttribute* attr = m_CurrentNode->first_attribute(name.c_str()))
			m_CurrentAttribute = attr;
		else ionerror(IOError);
	}

	bool XMLArchive::TryEnterAttribute(const String& name)
	{
		ionassert(!m_CurrentAttribute);
		XMLAttribute* attr = m_CurrentNode->first_attribute(name.c_str());
		if (attr)
			m_CurrentAttribute = attr;

		return attr;
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
		ionassert(m_XML);

		if (file.Open(EFileMode::Write | EFileMode::CreateNew)
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
		m_XML = xml;
		SeekRoot();
	}

	std::shared_ptr<XMLDocument> XMLArchive::SaveXML() const
	{
		return m_XML;
	}
}
