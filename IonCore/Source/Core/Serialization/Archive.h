#pragma once

#include "Core/Base.h"
#include "Core/File/File.h"
#include "StructSerializer.h"

namespace Ion
{
	class Archive;

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

	class IArrayItem
	{
	public:
		virtual void Serialize(Archive& ar) = 0;
		virtual size_t GetIndex() const = 0;
	};

	template<typename T>
	class TArrayItem : public IArrayItem
	{
	public:
		TArrayItem(T& item, size_t index = (size_t)-1) :
			m_Item(item),
			m_Index(index)
		{
		}

		virtual void Serialize(Archive& ar) override
		{
			ar << m_Item;
		}

		virtual size_t GetIndex() const override
		{
			return m_Index;
		}

	private:
		T& m_Item;
		size_t m_Index;
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

#pragma region LeftShift Operators

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

#pragma endregion

		virtual void Serialize(String& value) = 0;

		FORCEINLINE Archive& operator<<(String& value)
		{
			Serialize(value);
			return *this;
		}

		template<typename TEnum, TEnableIfT<TIsEnumV<TEnum>>* = 0>
		FORCEINLINE void SerializeEnum(TEnum& value)
		{
			if (IsText())
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
			else if (IsBinary())
			{
				auto utValue = (std::underlying_type_t<TEnum>)value;
				Serialize(utValue);
				value = (TEnum)utValue;
			}
		}

		template<typename T, TEnableIfT<TIsEnumV<T>>* = 0>
		FORCEINLINE Archive& operator<<(T& value)
		{
			SerializeEnum(value);
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

	protected:
		FORCEINLINE void SetFlag(EArchiveFlags::Type flag)
		{
			m_ArchiveFlags |= flag;
		}

		virtual void Serialize(IArrayItem& item) = 0;

		virtual size_t GetCollectionSize() const { return 0; }

	private:
		std::underlying_type_t<EArchiveFlags::Type> m_ArchiveFlags;

	public:
		// Generic array serialization
		template<typename T>
		friend FORCEINLINE Archive& operator<<(Archive& ar, TArray<T>& array)
		{
			size_t count = ar.IsSaving() ? array.size() : 0;
			if (ar.IsBinary())
			{
				ar << count;
			}
			else if (ar.IsLoading())
			{
				count = ar.GetCollectionSize();
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
					TArrayItem<T> arrayItem(element, n);
					ar.Serialize(arrayItem);
					array.emplace_back(Move(element));
				}
			}
			else if (ar.IsSaving())
			{
				for (size_t n = 0; n < count; ++n)
				{
					TArrayItem<T> arrayItem(array.at(n), n);
					ar.Serialize(arrayItem);
				}
			}

			return ar;
		}
	};

#define SERIALIZE_BIT_FIELD(ar, f) { bool bField = f; ar << bField; f = bField; }

}

namespace Ion::Test { void ArchiveTest(); }
