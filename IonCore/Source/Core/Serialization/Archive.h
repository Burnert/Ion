#pragma once

#include "Core/Base.h"
#include "Core/File/File.h"
#include "StructSerializer.h"

namespace Ion
{
	REGISTER_LOGGER(SerializationLogger, "Core::Serialization");

	enum class EArchiveType
	{
		Saving,
		Loading
	};

	class ION_API Archive
	{
	public:
		FORCEINLINE Archive(EArchiveType type) :
			m_ArchiveType(type)
		{
		}

		virtual void Serialize(void* const bytes, size_t size) = 0;

		template<typename T, TEnableIfT<std::is_fundamental_v<T>>* = 0>
		FORCEINLINE void Serialize(T& value)
		{
			Serialize(&value, sizeof(T));
		}

		virtual void Serialize(String& value) = 0;

		template<typename T, TEnableIfT<std::is_fundamental_v<T>>* = 0>
		FORCEINLINE Archive& operator<<(T& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(String& value)
		{
			Serialize(value);
			return *this;
		}

		virtual void LoadFromFile(File& file) = 0;
		virtual void SaveToFile(File& file) const = 0;

		FORCEINLINE void LoadFromFile(File&& file)
		{
			File f = Move(file);
			LoadFromFile(f);
		}

		FORCEINLINE void SaveToFile(File&& file) const
		{
			File f = Move(file);
			SaveToFile(f);
		}

		FORCEINLINE bool IsLoading() const
		{
			return m_ArchiveType == EArchiveType::Loading;
		}

		FORCEINLINE bool IsSaving() const
		{
			return m_ArchiveType == EArchiveType::Saving;
		}

	private:
		EArchiveType m_ArchiveType;
	};

	// Generic array serialization
	template<typename T>
	FORCEINLINE Archive& operator<<(Archive& ar, TArray<T>& array)
	{
		size_t count = ar.IsSaving() ? array.size() : 0;
		ar << count;

		// Clear and resize the array, so the memory to load into is available.
		if (ar.IsLoading())
		{
			array.clear();
			array.reserve(count);

			for (size_t n = 0; n < count; ++n)
			{
				// Don't default construct the element
				union {
					T element;
				};
				ar << element;
				array.emplace_back(Move(element));
			}
		}
		else if (ar.IsSaving())
		{
			for (T& element : array)
			{
				ar << element;
			}
		}

		return ar;
	}

	class ION_API BinaryArchive : public Archive
	{
	public:
		FORCEINLINE BinaryArchive(EArchiveType type) :
			Archive(type),
			m_Cursor(0)
		{
		}

		virtual void Serialize(void* const bytes, size_t size) override;
		virtual void Serialize(String& value) override;

		virtual void LoadFromFile(File& file) override;
		virtual void SaveToFile(File& file) const override;

	private:
		TArray<uint8> m_ByteArray;
		size_t m_Cursor;
	};
}

namespace Ion::Test { void ArchiveTest(); }
