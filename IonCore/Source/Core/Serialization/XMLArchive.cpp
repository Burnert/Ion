#include "Core/CorePCH.h"

#include "Core/Error/Error.h"
#include "Core/String/StringUtils.h"

#include "XMLArchive.h"

namespace Ion
{
	void XMLArchive::Serialize(void* const bytes, size_t size)
	{
		// @TODO: Encode the binary somehow (base64?)
	}

	void XMLArchive::Serialize(bool& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(int8& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(int16& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(int32& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(int64& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(uint8& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(uint16& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(uint32& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(uint64& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(float& value)
	{
		Serialize(ToString(value));
	}

	void XMLArchive::Serialize(double& value)
	{
		Serialize(ToString(value));
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
}
