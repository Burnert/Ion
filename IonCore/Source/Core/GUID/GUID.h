#pragma once

#include "Core/Base.h"
#include "Core/Error/Error.h"
#include "Core/Serialization/Archive.h"

namespace Ion
{
	using GUIDBytesArray = TFixedArray<uint8, 16>;

	/**
	 * @brief Globally Unique Identifier class
	 * 
	 * @details Don't call sizeof() on this class, if you want to
	 * get the size of the GUID bytes. This class has an additional
	 * String field in debug mode. Use GUID::Size instead.
	 * 
	 * @see GUID::Size
	 */
	class ION_API GUID
	{
	private:
		struct ZeroInitializerT { };
		struct InvalidInitializerT { };

	public:
		/* {00000000-0000-0000-0000-000000000000} */
		inline static constexpr ZeroInitializerT Zero = { };
		/* {ffffffff-ffff-ffff-ffff-ffffffffffff} */
		inline static constexpr InvalidInitializerT Invalid = { };

		GUID();
		GUID(const GUIDBytesArray& bytes);
		GUID(GUID::ZeroInitializerT);
		GUID(GUID::InvalidInitializerT);
		GUID(const GUID& other);
		GUID(GUID&& other) noexcept;

		static Result<GUID, StringConversionError> FromString(const String& guidStr);

		String ToString() const;
		bool IsZero() const;
		bool IsInvalid() const;
		/**
		 * @brief Is non-zero and not invalid
		 */
		bool IsApplicable() const;

		GUID& operator=(const GUID& other);
		GUID& operator=(GUID&& other) noexcept;
		bool operator==(const GUID& other) const;
		bool operator!=(const GUID& other) const;
		operator bool() const;

		void GetRawBytes(uint8(&outBytes)[16]) const;
		GUIDBytesArray GetRawBytes() const;

		void Swap(GUID& other);

	private:
		static Result<GUIDBytesArray, StringConversionError> PlatformGenerateGUIDFromString(const String& str);
		static GUIDBytesArray PlatformGenerateGUID();
		String PlatformGUIDToString() const;

	private:
		GUIDBytesArray m_Bytes;
#if ION_DEBUG
		mutable String m_AsString;
		void CacheString();
#endif
	public:
		static inline constexpr size_t Size = sizeof(m_Bytes);

		// Serialization
		FORCEINLINE friend Archive& operator<<(Archive& ar, GUID& guid)
		{
			if (ar.IsBinary())
			{
				ar.Serialize(&guid.m_Bytes, sizeof(guid.m_Bytes));
			}
			else if (ar.IsText())
			{
				String sGuid = ar.IsSaving() ? guid.ToString() : EmptyString;
				ar.Serialize(sGuid);
				if (ar.IsLoading())
					guid = GUID::FromString(sGuid).UnwrapOr(GUID::Zero);
			}
#if ION_DEBUG
			guid.CacheString();
#endif
			return ar;
		}
	};

	// Inline definitions

#if ION_DEBUG
#define CACHE_STRING() CacheString();
#else
#define CACHE_STRING()
#endif

	inline GUID::GUID() :
		m_Bytes(PlatformGenerateGUID())
	{
		CACHE_STRING()
	}

	inline GUID::GUID(const GUIDBytesArray& bytes) :
		m_Bytes(bytes)
	{
		CACHE_STRING()
	}

	inline GUID::GUID(GUID::ZeroInitializerT) :
		m_Bytes()
	{
		CACHE_STRING()
	}

	inline GUID::GUID(GUID::InvalidInitializerT) :
		m_Bytes()
	{
		m_Bytes.fill(0xFF);
		CACHE_STRING()
	}

	inline GUID::GUID(const GUID& other) :
		m_Bytes(other.m_Bytes)
	{
		CACHE_STRING()
	}

	inline GUID::GUID(GUID&& other) noexcept :
		m_Bytes(Move(other.m_Bytes))
	{
		CACHE_STRING()
	}

	inline Result<GUID, StringConversionError> GUID::FromString(const String& guidStr)
	{
		ionmatchresult(PlatformGenerateGUIDFromString(guidStr),
			mfwdthrow(StringConversionError)
			melse return GUID(R.Unwrap());
		)
	}

	inline String GUID::ToString() const
	{
		return PlatformGUIDToString();
	}

	inline bool GUID::IsZero() const
	{
		return !(((uint64*)&m_Bytes)[0] | ((uint64*)&m_Bytes)[1]);
	}

	inline bool GUID::IsInvalid() const
	{
		return (((uint64*)&m_Bytes)[0] + ((uint64*)&m_Bytes)[1] + 2) == 0;
	}

	inline bool GUID::IsApplicable() const
	{
		return !IsZero() && !IsInvalid();
	}

	inline GUID& GUID::operator=(const GUID& other)
	{
		m_Bytes = other.m_Bytes;
		CACHE_STRING()
		return *this;
	}

	inline GUID& GUID::operator=(GUID&& other) noexcept
	{
		m_Bytes = Move(other.m_Bytes);
		CACHE_STRING()
		return *this;
	}

	inline bool GUID::operator==(const GUID& other) const
	{
		// Endianness doesn't matter here, because we're just comparing bytes
		// and comparing two 64-bit numbers should be faster
		// than comparing sixteen 8-bit numbers.
		return
			((uint64*)&m_Bytes)[0] == ((uint64*)&other.m_Bytes)[0] &&
			((uint64*)&m_Bytes)[1] == ((uint64*)&other.m_Bytes)[1];
		// (but it actually isn't when optimized...)
		// (whatever, it looks cool)
	}

	inline bool GUID::operator!=(const GUID& other) const
	{
		return !operator==(other);
	}

	inline GUID::operator bool() const
	{
		return IsApplicable();
	}

	inline void GUID::GetRawBytes(uint8(&outBytes)[16]) const
	{
		memcpy(outBytes, &m_Bytes, sizeof(m_Bytes));
	}

	inline GUIDBytesArray GUID::GetRawBytes() const
	{
		return m_Bytes;
	}

	inline void GUID::Swap(GUID& other)
	{
		m_Bytes.swap(other.m_Bytes);
	}
}

template<>
struct std::hash<Ion::GUID>
{
	size_t operator()(const Ion::GUID& guid) const noexcept
	{
		uint8 bytes[16];
		guid.GetRawBytes(bytes);
		uint64 bytes64[2] = { *(uint64*)&bytes[0], *(uint64*)&bytes[8] };
		size_t h1 = THash<uint64>()(bytes64[0]);
		size_t h2 = THash<uint64>()(bytes64[1]);
		return h1 ^ h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
	}
};
