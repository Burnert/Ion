#pragma once

#include "Core/Base.h"
#include "Core/Error/Error.h"

struct StructTest
{
	uint64 A;
};

namespace Ion
{
	class IStructSerializer
	{
	public:
		virtual void GetBytes(void* outBuffer) const = 0;
		virtual size_t GetSize() const = 0;
	};

	template<typename T>
	class ION_API TStructSerializer : public IStructSerializer
	{
	public:
		TStructSerializer(T* pStruct);

		void PutBytes(const TFixedArray<uint8, sizeof(T)>& bytes) const;
		const TFixedArray<uint8, sizeof(T)>& GetBytes() const;

	private:
		T* m_Struct;

		friend class Storage;
	};

	template<typename T>
	FORCEINLINE TStructSerializer<T>::TStructSerializer(T* pStruct) :
		m_Struct(pStruct)
	{
		ionassert(pStruct);
	}

	template<typename T>
	FORCEINLINE void TStructSerializer<T>::PutBytes(const TFixedArray<uint8, sizeof(T)>& bytes) const
	{
		memcpy_s(m_Struct, sizeof(T), &bytes, sizeof(T));
	}

	template<typename T>
	FORCEINLINE const TFixedArray<uint8, sizeof(T)>& TStructSerializer<T>::GetBytes() const
	{
		return *reinterpret_cast<TFixedArray<uint8, sizeof(T)>*>(m_Struct);
	}
}
