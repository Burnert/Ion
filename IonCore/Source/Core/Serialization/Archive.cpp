#include "Core/CorePCH.h"

#include "Archive.h"

namespace Ion
{
	void BinaryArchive::SerializeBytes(void* const bytes, size_t size)
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
