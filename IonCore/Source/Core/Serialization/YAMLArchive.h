#pragma once

#include "Archive.h"

#include "Core/Memory/RefCount.h"
#include "Core/File/YAML.h"

#define ON_YAML_AR(ar) if (YAMLArchive* yml = dynamic_cast<YAMLArchive*>(&ar))
#define IS_YAML_AR(ar) !!dynamic_cast<YAMLArchive*>(&ar)

namespace Ion
{
	struct YAMLNodeData
	{
		size_t NodeIndex;
	};

	class ION_API YAMLArchive : public Archive
	{
	public:
		YAMLArchive(EArchiveType type);

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

		virtual void LoadFromFile(File& file) override;
		virtual void SaveToFile(File& file) const override;

		virtual ArchiveNode EnterRootNode() override;
		virtual ArchiveNode EnterNode(const ArchiveNode& parentNode, StringView name, EArchiveNodeType type) override;
		virtual ArchiveNode EnterNextNode(const ArchiveNode& currentNode, EArchiveNodeType type) override;

		virtual void UseNode(const ArchiveNode& node) override;

		void EnterNode(const String& name);
		void ExitNode();

		void BeginSeq();
		bool IterateSeq();
		void EndSeq();

	protected:
		virtual void Serialize(ArchiveArrayItem& item) override;

		virtual size_t GetCollectionSize() const override;

	private:
		template<typename T, TEnableIf<std::is_fundamental_v<T>>* = 0>
		void SerializeFundamental(T& value);

		static const YAMLNodeData& GetYAMLNodeDataFromArchiveNode(const ArchiveNode& node);

		static ryml::NodeType ArchiveNodeTypeToYAMLNodeType(EArchiveNodeType type);

		void InitYAMLNode(EArchiveNodeType type, size_t node, ryml::csubstr key, ryml::csubstr val, bool bKey);

		/**
		 * @brief Called by every type of serialize function in the end.
		 */
		template<typename T>
		void Serialize_Private(T& value);

	private:
		TSharedPtr<ryml::Tree> m_YAMLTree;
		ryml::NodeRef m_CurrentNode;
		size_t m_CurrentNodeIndex;
		// @TODO: This should be a stack of indices (so there can be nested sequences)
		mutable size_t m_SeqIndex;
	};

	template<typename T, TEnableIf<std::is_fundamental_v<T>>*>
	FORCEINLINE void YAMLArchive::SerializeFundamental(T& value)
	{
		Serialize_Private(value);
	}

	template<typename T>
	FORCEINLINE void YAMLArchive::Serialize_Private(T& value)
	{
		ionassert(m_YAMLTree);

		if (IsSaving())
		{
			ryml::csubstr str = m_YAMLTree->to_arena(value);
			m_YAMLTree->set_val(m_CurrentNodeIndex, str);
		}
		else if (IsLoading())
		{
			ryml::csubstr str = m_YAMLTree->val(m_CurrentNodeIndex);
			if constexpr (TIsFloatingV<T>)
			{
				ryml::from_chars_float(str, &value);
			}
			else
			{
				ryml::from_chars(str, &value);
			}
		}
	}

	FORCEINLINE const YAMLNodeData& YAMLArchive::GetYAMLNodeDataFromArchiveNode(const ArchiveNode& node)
	{
		return node.GetCustomData<YAMLNodeData>();
	}

	FORCEINLINE ryml::NodeType YAMLArchive::ArchiveNodeTypeToYAMLNodeType(EArchiveNodeType type)
	{
		switch (type)
		{
		case EArchiveNodeType::None:  return ryml::NOTYPE;
		case EArchiveNodeType::Value: return ryml::VAL;
		case EArchiveNodeType::Map:   return ryml::MAP;
		case EArchiveNodeType::Seq:   return ryml::SEQ;
		}
		return ryml::NOTYPE;
	}
}
