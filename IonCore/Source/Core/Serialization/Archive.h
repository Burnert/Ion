#pragma once

#include "Core/Base.h"
#include "Core/File/File.h"
#include "StructSerializer.h"

namespace Ion
{
	REGISTER_LOGGER(SerializationLogger, "Core::Serialization");

	class Archive;

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

#pragma region Node Based Archive

	enum class EArchiveNodeType : uint8
	{
		None,
		Value,
		Map,
		Seq,
	};

	struct ArchiveNode
	{
		Archive* Ar;
		// @TODO: This can't be a string. Make it a string view? (str owned by archive)
		String Name;
		EArchiveNodeType Type;

		uint8 CustomData[8];

		ArchiveNode(Archive* ar, StringView name, EArchiveNodeType type) :
			Ar(ar),
			Name(name),
			Type(type),
			CustomData()
		{
			ionassert(ar);
			ionassert(type != EArchiveNodeType::None);
		}

		/**
		 * @brief Empty node initializer.
		 * 
		 * @details Node can still be used to serialize data even if the archive does not recognize nodes.
		 */
		ArchiveNode(Archive* ar) :
			Ar(ar),
			Name(""),
			Type(EArchiveNodeType::None),
			CustomData()
		{
		}

		/**
		 * @brief Invalid node initializer.
		 */
		ArchiveNode() :
			Ar(nullptr),
			Name(""),
			Type(EArchiveNodeType::None),
			CustomData()
		{
		}

		template<typename T>
		FORCEINLINE const T& GetCustomData() const
		{
			return *reinterpret_cast<const T*>(CustomData);
		}

		template<typename T>
		FORCEINLINE void SetCustomData(const T& data)
		{
			*reinterpret_cast<T*>(CustomData) = data;
		}

		FORCEINLINE bool IsValid() const
		{
			return Ar && Type != EArchiveNodeType::None;
		}

		FORCEINLINE operator bool() const
		{
			return IsValid();
		}

		template<typename T>
		FORCEINLINE void operator<<(T& value)
		{
			ionassert(Ar);
			// @TODO: This assert triggers on empty nodes, which normally works
			// on archives that don't support nodes, but on the others it would be
			// benefitial to know if you're serializing using a wrong node type.
			//ionassert(Type == EArchiveNodeType::Value);

			Ar->UseNode(*this);
			*Ar << value;
		}
	};

#pragma endregion

#pragma region Array Serialization Helpers

	class ArchiveArrayItem
	{
	public:
		ArchiveArrayItem(size_t index) :
			m_Index(index)
		{
		}

		virtual void Serialize(Archive& ar) = 0;

		inline size_t GetIndex() const
		{
			return m_Index;
		}

	protected:
		size_t m_Index;
	};

	template<typename T>
	class TArchiveArrayItem : public ArchiveArrayItem
	{
	public:
		TArchiveArrayItem(T& item, size_t index = (size_t)-1) :
			ArchiveArrayItem(index),
			m_Item(item)
		{
		}

		virtual void Serialize(Archive& ar) override
		{
			ar << m_Item;
		}

	private:
		T& m_Item;
	};

#pragma endregion

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

		virtual void Serialize(String& value) = 0;

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

		virtual ArchiveNode EnterRootNode() = 0;
		virtual ArchiveNode EnterNode(const ArchiveNode& parentNode, StringView name, EArchiveNodeType type) = 0;
		virtual ArchiveNode EnterNextNode(const ArchiveNode& currentNode, EArchiveNodeType type) = 0;

		/**
		 * @brief From this point on, use the specified node when performing 
		 * serialization operations. (Serialize or operator<<)
		 * 
		 * @param node Node to use
		 */
		virtual void UseNode(const ArchiveNode& node) = 0;

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

		FORCEINLINE Archive& operator<<(String& value)
		{
			Serialize(value);
			return *this;
		}

		template<typename T, TEnableIfT<TIsEnumV<T>>* = 0>
		FORCEINLINE Archive& operator<<(T& value)
		{
			SerializeEnum(value);
			return *this;
		}

#pragma endregion

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

		virtual void Serialize(ArchiveArrayItem& item) = 0;

		FORCEINLINE Archive& operator<<(ArchiveArrayItem& item)
		{
			Serialize(item);
			return *this;
		}

		virtual size_t GetCollectionSize() const { return 0; }

	private:
		union
		{
			std::underlying_type_t<EArchiveFlags::Type> m_ArchiveFlags;
			struct
			{
				uint8 m_bSaving : 1;
				uint8 m_bLoading : 1;
				uint8 m_bBinary : 1;
				uint8 m_bText : 1;
			};
		};

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

			if (ar.IsLoading())
			{
				// Clear and resize the array, so the memory to load into is available.
				array.clear();
				array.reserve(count);

				for (size_t n = 0; n < count; ++n)
				{
					// Don't default construct the element
					union { T element; };
					ar << TArchiveArrayItem(element, n);
					array.emplace_back(Move(element));
				}
			}
			else if (ar.IsSaving())
			{
				for (size_t n = 0; n < count; ++n)
				{
					ar << TArchiveArrayItem(array.at(n), n);
				}
			}

			return ar;
		}
	};

#define SERIALIZE_BIT_FIELD(ar, f) { bool bField = f; ar << bField; f = bField; }

}

namespace Ion::Test { void ArchiveTest(); }
