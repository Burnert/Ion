#pragma once

namespace Ion
{
	using GUIDBytesArray = TFixedArray<uint8, 16>;

	class ION_API GUID
	{
	public:
		GUID();
		GUID(const String& guidStr);
		GUID(const GUID& other);
		GUID(GUID&& other) noexcept;

		String ToString() const;

		GUID& operator=(const GUID& other);
		GUID& operator=(GUID&& other) noexcept;
		bool operator==(const GUID& other) const;
		bool operator!=(const GUID& other) const;

		void GetRawBytes(uint8(&outBytes)[16]) const;
		GUIDBytesArray GetRawBytes() const;

	private:
		void PlatformGenerateGUID();
		void PlatformGenerateGUIDFromString(const String& str);
		String PlatformGUIDToString() const;

	private:
		uint8 m_Bytes[16];
	};

	inline GUID::GUID() :
		m_Bytes()
	{
		PlatformGenerateGUID();
	}

	inline GUID::GUID(const String& guidStr) :
		m_Bytes()
	{
		PlatformGenerateGUIDFromString(guidStr);
	}

	inline GUID::GUID(const GUID& other)
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(GUID));
	}

	inline GUID::GUID(GUID&& other) noexcept
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(GUID));
		memset(other.m_Bytes, 0, sizeof(other.m_Bytes));
	}

	inline String GUID::ToString() const
	{
		return PlatformGUIDToString();
	}

	inline GUID& GUID::operator=(const GUID& other)
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(GUID));
		return *this;
	}

	inline GUID& GUID::operator=(GUID&& other) noexcept
	{
		memcpy(m_Bytes, other.m_Bytes, sizeof(GUID));
		memset(other.m_Bytes, 0, sizeof(other.m_Bytes));
		return *this;
	}

	inline bool GUID::operator==(const GUID& other) const
	{
		for (int32 i = 0; i < sizeof(GUID); ++i)
		{
			if (m_Bytes[i] != other.m_Bytes[i])
				return false;
		}
		return true;
	}

	inline bool GUID::operator!=(const GUID& other) const
	{
		return !operator==(other);
	}

	inline void GUID::GetRawBytes(uint8(&outBytes)[16]) const
	{
		memcpy(outBytes, m_Bytes, sizeof(GUID));
	}

	inline GUIDBytesArray GUID::GetRawBytes() const
	{
		GUIDBytesArray bytes;
		memcpy(bytes.data(), m_Bytes, sizeof(GUID));

		return bytes;
	}
}
