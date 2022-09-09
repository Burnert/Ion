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
			SetFlag(EArchiveFlags::Text);
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
		bool TryEnterSiblingNode();
		void ExitNode();

		void EnterAttribute(const String& name);
		bool TryEnterAttribute(const String& name);
		void ExitAttribute();

		void SeekRoot();

		virtual void LoadFromFile(File& file) override;
		virtual void SaveToFile(File& file) const override;

		void LoadXML(const std::shared_ptr<XMLDocument>& xml);
		std::shared_ptr<XMLDocument> SaveXML() const;
		
	private:
		template<typename T, TEnableIf<std::is_fundamental_v<T>>* = 0>
		FORCEINLINE void SerializeFundamental(T& value)
		{
			if (IsSaving())
			{
				Serialize(ToString(value));
			}
			else if (IsLoading())
			{
				String sValue;
				Serialize(sValue);

				if (TOptional<T> opt = TStringParser<T>()(sValue))
				{
					value = *opt;
				}
			}
		}

	private:
		std::shared_ptr<XMLDocument> m_XML;
		XMLNode* m_CurrentNode;
		XMLAttribute* m_CurrentAttribute;
	};
}
