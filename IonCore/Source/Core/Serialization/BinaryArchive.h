#pragma once

#include "Archive.h"

namespace Ion
{
	class ION_API BinaryArchive : public Archive
	{
	public:
		FORCEINLINE BinaryArchive(EArchiveType type) :
			Archive(type),
			m_Cursor(0)
		{
			SetFlag(EArchiveFlags::Binary);
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

		virtual void Serialize(ArchiveArrayItem& item) override;

		virtual void LoadFromFile(File& file) override;
		virtual void SaveToFile(File& file) const override;

		virtual ArchiveNode EnterRootNode() override;
		virtual ArchiveNode EnterNode(const ArchiveNode& parentNode, StringView name, EArchiveNodeType type) override;
		virtual ArchiveNode EnterNextNode(const ArchiveNode& currentNode, EArchiveNodeType type) override;
		virtual void UseNode(const ArchiveNode& node) override;
		virtual ArchiveNode GetCurrentNode() override;
	private:
		TArray<uint8> m_ByteArray;
		size_t m_Cursor;
	};
}
