#pragma once

#include "Archive.h"
#include "Core/File/XML.h"
#include "Core/String/StringParser.h"

namespace Ion
{
	class ION_API XMLArchive : public Archive
	{
	public:
		FORCEINLINE XMLArchive(EArchiveType type) :
			Archive(type),
			m_CurrentNode(nullptr),
			m_CurrentAttribute(nullptr)
		{
			SetFlag(EArchiveFlags::Text);
			if (type == EArchiveType::Saving)
			{
				m_XML = std::make_shared<XMLDocument>();
			}
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

		virtual void Serialize(ArchiveArrayItem& item) override { };

		template<typename TEnum, TEnableIfT<TIsEnumV<TEnum>>* = 0>
		FORCEINLINE void SerializeEnum(TEnum& value)
		{
			String sEnum = IsSaving() ? TEnumParser<TEnum>::ToString(value) : EmptyString;
			Serialize(sEnum);
			if (IsLoading())
			{
				TOptional<TEnum> opt = TEnumParser<TEnum>::FromString(sEnum);
				if (opt); else
				{
					SerializationLogger.Error("Cannot parse enum value. {} -> {}", sEnum, typeid(TEnum).name());
					return;
				}
				value = *opt;
			}
		}

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

		virtual ArchiveNode EnterRootNode() override;
		virtual ArchiveNode EnterNode(const ArchiveNode& parentNode, StringView name, EArchiveNodeType type) override;
		virtual ArchiveNode EnterNextNode(const ArchiveNode& currentNode, EArchiveNodeType type) override;
		virtual void UseNode(const ArchiveNode& node) override;
		virtual ArchiveNode GetCurrentNode() override;

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

	class XMLArchiveAdapter
	{
	public:
		XMLArchiveAdapter(Archive& ar);

		void EnterNode(const String& name);
		bool TryEnterNode(const String& name);
		bool TryEnterSiblingNode();
		void ExitNode();

		void EnterAttribute(const String& name);
		bool TryEnterAttribute(const String& name);
		void ExitAttribute();

		void SeekRoot();

		template<typename T>
		FORCEINLINE void Serialize(T& value)
		{
			m_Archive.Serialize(value);
		}

		template<typename T, TEnableIfT<!TIsEnumV<T>>* = 0>
		FORCEINLINE Archive& operator&=(T& value)
		{
			m_Archive &= value;
			return m_Archive;
		}

		template<typename TEnum, TEnableIfT<TIsEnumV<TEnum>>* = 0>
		FORCEINLINE void SerializeEnum(TEnum& value)
		{
			if (XMLArchive* ar = AsXMLArchive())
			{
				ar->SerializeEnum(value);
			}
			else
			{
				auto utValue = (std::underlying_type_t<TEnum>)value;
				m_Archive &= utValue;
				value = (TEnum)utValue;
			}
		}

		template<typename T, TEnableIfT<TIsEnumV<T>>* = 0>
		FORCEINLINE Archive& operator&=(T& value)
		{
			SerializeEnum(value);
			return m_Archive;
		}

		FORCEINLINE Archive& GetUnderlyingArchive() const
		{
			return m_Archive;
		}

	private:
		XMLArchive* AsXMLArchive() const;

	private:
		Archive& m_Archive;
	};
}
