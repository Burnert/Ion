#include "Core/CorePCH.h"

#include "BinaryArchive.h"

namespace Ion
{
	void BinaryArchive::Serialize(void* const bytes, size_t size)
	{
		if (IsLoading())
		{
			// Copy the bytes from the archive into the destination.
			ionverify((m_Cursor + size) <= m_ByteArray.size());
			void* memory = &m_ByteArray[m_Cursor];
			memcpy_s(bytes, size, memory, size);
			m_Cursor += size;
		}
		else if (IsSaving())
		{
			// Copy the bytes from the source into the archive.
			size_t newSize = m_ByteArray.size() + size;
			m_ByteArray.resize(newSize);
			memcpy_s(&m_ByteArray[m_Cursor], size, bytes, size);
			m_Cursor += size;
		}
	}

	void BinaryArchive::Serialize(bool& value)
	{
		Serialize(&value, sizeof(bool));
	}

	void BinaryArchive::Serialize(int8& value)
	{
		Serialize(&value, sizeof(int8));
	}

	void BinaryArchive::Serialize(int16& value)
	{
		Serialize(&value, sizeof(int16));
	}

	void BinaryArchive::Serialize(int32& value)
	{
		Serialize(&value, sizeof(int32));
	}

	void BinaryArchive::Serialize(int64& value)
	{
		Serialize(&value, sizeof(int64));
	}

	void BinaryArchive::Serialize(uint8& value)
	{
		Serialize(&value, sizeof(uint8));
	}

	void BinaryArchive::Serialize(uint16& value)
	{
		Serialize(&value, sizeof(uint16));
	}

	void BinaryArchive::Serialize(uint32& value)
	{
		Serialize(&value, sizeof(uint32));
	}

	void BinaryArchive::Serialize(uint64& value)
	{
		Serialize(&value, sizeof(uint64));
	}

	void BinaryArchive::Serialize(float& value)
	{
		Serialize(&value, sizeof(float));
	}

	void BinaryArchive::Serialize(double& value)
	{
		Serialize(&value, sizeof(double));
	}

	void BinaryArchive::Serialize(String& value)
	{
		if (IsLoading())
		{
			ionverify(m_Cursor <= m_ByteArray.size());
			char* start = (char*)&m_ByteArray[m_Cursor];
			// Make sure we don't go past the end of the array using strnlen_s
			size_t maxLength = m_ByteArray.size() - m_Cursor;
			size_t copyLength = strnlen_s(start, maxLength);
			value.resize(copyLength, 0);
			memcpy_s(value.data(), copyLength, start, copyLength);
			// Add 1 to go past the terminating 0 (unless it's the end of the file).
			m_Cursor += std::min(copyLength + 1, maxLength);
		}
		else if (IsSaving())
		{
			ionassert(strlen(value.c_str()) == value.size());
			// Save with the null character
			Serialize(value.data(), value.size() + 1);
		}
	}

	void BinaryArchive::Serialize(IArrayItem& item)
	{
		item.Serialize(*this);
	}

	void BinaryArchive::LoadFromFile(File& file)
	{
		ionassert(IsLoading());
		ionassert(!file.IsOpen());

		if (file.Open(EFileMode::Read)
			.Err([](Error& err)
			{
				SerializationLogger.Error("Cannot load Binary Archive from file.\n{}", err.Message);
			}))
		{
			int64 fileSize = file.GetSize();
			m_ByteArray.resize(fileSize);
			file.Read(m_ByteArray.data(), fileSize);
			m_Cursor = 0;
			file.Close();
		}
	}

	void BinaryArchive::SaveToFile(File& file) const
	{
		ionassert(IsSaving());
		ionassert(!file.IsOpen());

		if (file.Open(EFileMode::Write | EFileMode::CreateNew)
			.Err([](Error& err)
			{
				SerializationLogger.Error("Cannot save Binary Archive to file.\n{}", err.Message);
			}))
		{
			file.Write(m_ByteArray.data(), m_ByteArray.size());

		}
	}
}
