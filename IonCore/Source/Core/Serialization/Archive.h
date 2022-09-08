#pragma once

#include "Core/Base.h"
#include "Core/File/File.h"
#include "StructSerializer.h"

namespace Ion
{
	REGISTER_LOGGER(SerializationLogger, "Core::Serialization");

	namespace EArchiveFlags
	{
		enum Type : uint8
		{
			None    = 0,
			Saving  = 1 << 0,
			Loading = 1 << 1,
			Binary  = 1 << 2,
			Text    = 1 << 3,
		};
	}

	enum class EArchiveType
	{
		Saving,
		Loading,
	};

	class ION_API Archive
	{
	public:
		FORCEINLINE Archive(EArchiveType type)
		{
			m_ArchiveFlags =
				FlagsIf(type == EArchiveType::Loading, EArchiveFlags::Loading) |
				FlagsIf(type == EArchiveType::Saving,  EArchiveFlags::Saving);
		}

		virtual void Serialize(void* const bytes, size_t size) = 0;

		virtual void Serialize(bool& value) = 0;
		virtual void Serialize(int8& value) = 0;
		virtual void Serialize(int16& value) = 0;
		virtual void Serialize(int32& value) = 0;
		virtual void Serialize(int64& value) = 0;
		virtual void Serialize(uint8& value) = 0;
		virtual void Serialize(uint16& value) = 0;
		virtual void Serialize(uint32& value) = 0;
		virtual void Serialize(uint64& value) = 0;
		virtual void Serialize(float& value) = 0;
		virtual void Serialize(double& value) = 0;

		FORCEINLINE Archive& operator<<(bool& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(int8& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(int16& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(int32& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(int64& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(uint8& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(uint16& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(uint32& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(uint64& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(float& value)
		{
			Serialize(value);
			return *this;
		}

		FORCEINLINE Archive& operator<<(double& value)
		{
			Serialize(value);
			return *this;
		}

		virtual void Serialize(String& value) = 0;

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
			return m_ArchiveFlags & EArchiveFlags::Loading;
		}

		FORCEINLINE bool IsSaving() const
		{
			return m_ArchiveFlags & EArchiveFlags::Saving;
		}

		FORCEINLINE bool IsBinary() const
		{
			return m_ArchiveFlags & EArchiveFlags::Binary;
		}

		FORCEINLINE bool IsText() const
		{
			return m_ArchiveFlags & EArchiveFlags::Text;
		}

	private:
		std::underlying_type_t<EArchiveFlags::Type> m_ArchiveFlags;
	};

#define SERIALIZE_BIT_FIELD(ar, f) { bool bField = f; ar << bField; f = bField; }

	class XMLArchive;

	class XMLArchiveAdapter
	{
	public:
		FORCEINLINE XMLArchiveAdapter(Archive& ar) :
			m_Archive(&ar)
		{
		}

		template<typename T>
		FORCEINLINE void Serialize(T& value)
		{
			m_Archive->Serialize(value);
		}

		void EnterNode(const String& name);
		bool TryEnterNode(const String& name);
		void ExitNode();

		void EnterAttribute(const String& name);
		bool TryEnterAttribute(const String& name);
		void ExitAttribute();

	private:
		XMLArchive* AsXMLArchive();

	private:
		Archive* m_Archive;
	};

	// Generic array serialization
	template<typename T>
	FORCEINLINE Archive& operator<<(Archive& ar, TArray<T>& array)
	{
		size_t count = ar.IsSaving() ? array.size() : 0;
		if (ar.IsBinary())
		{
			ar << count;
		}

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
}

namespace Ion::Test { void ArchiveTest(); }
