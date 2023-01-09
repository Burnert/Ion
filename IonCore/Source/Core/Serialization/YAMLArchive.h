#pragma once

#include "Archive.h"

#include "Core/Memory/RefCount.h"
#include "Core/File/YAML.h"

namespace Ion
{
	class ION_API YAMLArchive : public Archive
	{
	public:
		FORCEINLINE YAMLArchive(EArchiveType type) :
			Archive(type),
			m_YAMLTree(nullptr)
		{
			SetFlag(EArchiveFlags::Text);
			if (type == EArchiveType::Saving)
			{
				m_YAMLTree = MakeShared<ryml::Tree>();
				m_CurrentNode = m_YAMLTree->rootref();
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

		virtual void LoadFromFile(File& file) override;
		virtual void SaveToFile(File& file) const override;

		void EnterNode(const String& name);
		void ExitNode();

		void BeginSeq();
		void EndSeq();

	protected:
		virtual void Serialize(IArrayItem& item) override;

		virtual size_t GetCollectionSize() const override;

	private:
		template<typename T, TEnableIf<std::is_fundamental_v<T>>* = 0>
		FORCEINLINE void SerializeFundamental(T& value)
		{
			ionassert(m_YAMLTree);
			ionassert(m_CurrentNode.valid());

			if (IsSaving())
			{
				m_CurrentNode << value;
			}
			else if (IsLoading())
			{
				m_CurrentNode >> value;
			}
		}

	private:
		TSharedPtr<ryml::Tree> m_YAMLTree;
		ryml::NodeRef m_CurrentNode;
	};
}
