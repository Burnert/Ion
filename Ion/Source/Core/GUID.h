#pragma once

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
		explicit GUID(const uint8 bytes[16]);
		explicit GUID(const GUIDBytesArray& bytes);
		explicit GUID(const String& guidStr);
		GUID(GUID::ZeroInitializerT);
		GUID(GUID::InvalidInitializerT);
		GUID(const GUID& other);
		GUID(GUID&& other) noexcept;

		String ToString() const;
		bool IsZero() const;
		bool IsInvalid() const;

		GUID& operator=(const GUID& other);
		GUID& operator=(GUID&& other) noexcept;
		bool operator==(const GUID& other) const;
		bool operator!=(const GUID& other) const;
		operator bool() const;

		void GetRawBytes(uint8(&outBytes)[16]) const;
		GUIDBytesArray GetRawBytes() const;

	private:
		void PlatformGenerateGUID();
		void PlatformGenerateGUIDFromString(const String& str);
		String PlatformGUIDToString() const;

	private:
		uint8 m_Bytes[16];
#if ION_DEBUG
		String m_AsString;
		void CacheString();
#endif
	public:
		static inline constexpr size_t Size = sizeof(m_Bytes);
	};

	// Inline definitions

#if ION_DEBUG
#define CACHE_STRING() CacheString();
#else
#define CACHE_STRING()
#endif

	inline GUID::GUID() :
		m_Bytes()
	{
		PlatformGenerateGUID();
		CACHE_STRING()
	}

	inline GUID::GUID(const uint8 bytes[16]) :
		m_Bytes()
	{
		memcpy(m_Bytes, bytes, sizeof(m_Bytes));
		CACHE_STRING()
	}

	inline GUID::GUID(const GUIDBytesArray& bytes) :
		m_Bytes()
	{
		memcpy(m_Bytes, &bytes, sizeof(bytes));
		CACHE_STRING()
	}

	inline GUID::GUID(const String& guidStr) :
		m_Bytes()
	{
		PlatformGenerateGUIDFromString(guidStr);
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
		memset(m_Bytes, 0xFF, sizeof(m_Bytes));
		CACHE_STRING()
	}

	inline GUID::GUID(const GUID& other)
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(m_Bytes));
		CACHE_STRING()
	}

	inline GUID::GUID(GUID&& other) noexcept
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(m_Bytes));
		memset(other.m_Bytes, 0, sizeof(other.m_Bytes));
		CACHE_STRING()
	}

	inline String GUID::ToString() const
	{
		return PlatformGUIDToString();
	}

	inline bool GUID::IsZero() const
	{
		return !(((uint64*)m_Bytes)[0] | ((uint64*)m_Bytes)[1]);
	}

	inline bool GUID::IsInvalid() const
	{
		return (((uint64*)m_Bytes)[0] + ((uint64*)m_Bytes)[1] + 2) == 0;
	}

	inline GUID& GUID::operator=(const GUID& other)
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(m_Bytes));
		CACHE_STRING()
		return *this;
	}

	inline GUID& GUID::operator=(GUID&& other) noexcept
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(m_Bytes));
		memset(other.m_Bytes, 0, sizeof(other.m_Bytes));
		CACHE_STRING()
		return *this;
	}

	inline bool GUID::operator==(const GUID& other) const
	{
		// Endianness doesn't matter here, because we're just comparing bytes
		// and comparing two 64-bit numbers should be faster
		// than comparing sixteen 8-bit numbers.
		return
			((uint64*)m_Bytes)[0] == ((uint64*)other.m_Bytes)[0] &&
			((uint64*)m_Bytes)[1] == ((uint64*)other.m_Bytes)[1];
		// (but it actually isn't when optimized...)
		// (whatever, it looks cool)
	}

	inline bool GUID::operator!=(const GUID& other) const
	{
		return !operator==(other);
	}

	inline GUID::operator bool() const
	{
		return !IsZero() && !IsInvalid();
	}

	inline void GUID::GetRawBytes(uint8(&outBytes)[16]) const
	{
		memcpy(outBytes, m_Bytes, sizeof(m_Bytes));
	}

	inline GUIDBytesArray GUID::GetRawBytes() const
	{
		GUIDBytesArray bytes;
		memcpy(bytes.data(), m_Bytes, sizeof(m_Bytes));

		return bytes;
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
