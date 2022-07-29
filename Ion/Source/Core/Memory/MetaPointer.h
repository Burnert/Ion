#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreMacros.h"

#ifndef _WIN64
#error Cannot compile MetaPointers for non-64-bit platforms.
#endif

#define ENABLE_META_POINTER 1

template<typename T>
struct TMetaPointer
{
	using MetaPointerType = TMetaPointer<T>;

	inline TMetaPointer(T* ptr) :
		m_PtrPart((int64)ptr),
		m_MetaBits(0)
	{
	}

	NODISCARD inline T* Get() const
	{
		return (T*)m_PtrPart;
	}

	inline void Set(T* ptr)
	{
		m_PtrPart = (int64)ptr;
	}

	template<typename CastT, size_t Offset>
	NODISCARD inline CastT& Meta()
	{
		static_assert(sizeof(CastT) + Offset <= 16);

		return *(CastT*)(((uint8*)&m_MetaBits) + Offset);
	}

	NODISCARD inline uint16& GetMetaBits()
	{
		return m_MetaBits;
	}

	NODISCARD inline uint16 GetMetaBitsValue() const
	{
		return m_MetaBits;
	}

	template<size_t Flag>
	inline void SetMetaFlag(bool bValue)
	{
		static_assert(Flag < 16);

		uint16 bit  = 1 << (15 - Flag);
		uint16 mask = ~bit;
		m_MetaBits  = (m_MetaBits & mask) | (bValue ? bit : 0);
	}

	template<size_t Flag>
	NODISCARD inline bool GetMetaFlag() const
	{
		return (m_MetaBits >> (15 - Flag)) & 1;
	}

	inline operator bool() const
	{
		return (bool)Get();
	}

	inline operator T*()
	{
		return Get();
	}

	inline T& operator*()
	{
		return *Get();
	}

	inline T* operator->()
	{
		return Get();
	}

	inline MetaPointerType& operator=(T* ptr)
	{
		Set(ptr);
		return *this;
	}

	inline bool operator==(T* ptr) const
	{
		return (T*)m_PtrPart == ptr;
	}

	inline bool operator==(nullptr_t) const
	{
		return m_PtrPart == 0;
	}

	inline bool operator!=(T* ptr) const
	{
		return (T*)m_PtrPart != ptr;
	}

	inline bool operator!=(nullptr_t) const
	{
		return m_PtrPart != 0;
	}

	inline bool operator==(const MetaPointerType& other) const
	{
		return m_PtrPart == other.m_PtrPart && m_MetaBits == other.m_MetaBits;
	}

	inline bool operator!=(const MetaPointerType& other) const
	{
		return !operator==(other);
	}

private:
#if ENABLE_META_POINTER
	int64 m_PtrPart : 48;
	uint64 m_MetaBits : 16;
#else
	int64 m_PtrPart;
	uint16 m_MetaBits;
#endif
};
