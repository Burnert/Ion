#pragma once

#include "Archive.h"
#include "Core/File/XML.h"

namespace Ion
{
	class XMLArchive : public Archive
	{
	public:
		FORCEINLINE XMLArchive(EArchiveType type) :
			Archive(type),
			m_CurrentNode(nullptr),
			m_CurrentAttribute(nullptr)
		{
		}

		virtual void Serialize(void* const bytes, size_t size) override;

		virtual void Serialize(bool& value) override;
		virtual void Serialize(int8& value) override;
		virtual void Serialize(int16& value) override;
		virtual void Serialize(int32& value) override;
		virtual void Serialize(int64& value) override;
		virtual void Serialize(uint8& value) override;
		virtual void Serialize(uint16& value) override;
		virtual void Serialize(uint32& value) override;
		virtual void Serialize(uint64& value) override;
		virtual void Serialize(float& value) override;
		virtual void Serialize(double& value) override;

		virtual void Serialize(String& value) override;

		void EnterNode(const String& name);
		bool TryEnterNode(const String& name);
		void ExitNode();

		void EnterAttribute(const String& name);
		bool TryEnterAttribute(const String& name);
		void ExitAttribute();

		virtual void LoadFromFile(File& file) override;
		virtual void SaveToFile(File& file) const override;

	private:
		std::unique_ptr<XMLDocument> m_XML;
		XMLNode* m_CurrentNode;
		XMLAttribute* m_CurrentAttribute;
	};
}
