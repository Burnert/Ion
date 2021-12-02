#include "IonPCH.h"

#include "AssetMemory.h"

#pragma warning(disable:6387)

namespace Ion
{
	AssetMemoryPool::AssetMemoryPool() :
		m_Size(0),
		m_Alignment(0),
		m_NextOffset(0),
		m_Data(nullptr)
	{ }

	void AssetMemoryPool::AllocatePool(size_t size, size_t alignment)
	{
		TRACE_FUNCTION();

		ionassert(!m_Data);
		ionassert(size);
		ionassert(Math::IsPowerOfTwo(alignment));

		m_Size = size;
		m_Alignment = alignment;
		m_NextOffset = 0;
		m_Data = _aligned_malloc(size, alignment);
	}

	void AssetMemoryPool::FreePool()
	{
		TRACE_FUNCTION();

		ionassert(m_Data);

		_aligned_free(m_Data);
		m_Size = 0;
		m_Alignment = 0;
		m_NextOffset = 0;
		m_Data = nullptr;
	}

	void AssetMemoryPool::ReallocPool(size_t newSize)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(newSize);
		ionassert(newSize > m_NextOffset);

		void* data = _aligned_malloc(newSize, m_Alignment);
		size_t size = Math::Min(m_Size, newSize);
		memcpy(data, m_Data, size);

		_aligned_free(m_Data);
		m_Size = size;
		m_Data = data;
	}

	void* AssetMemoryPool::Alloc(size_t size)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(size);

		uint8* current = (uint8*)m_Data + m_NextOffset;

		// Round up to m_Alignment
		size_t offsetToNext = AlignAs(size, m_Alignment);
		m_NextOffset += offsetToNext;

		// @TODO: Reallocate if the offset goes out of the buffer
		if (m_NextOffset > m_Size)
		{
			return nullptr;
		}

		// Save the sub-block
		AssetAllocData allocData { };
		allocData.Ptr = current;
		allocData.Size = size;
		m_AllocData.emplace(current, Move(allocData));

		return current;
	}

	void AssetMemoryPool::Free(void* data)
	{
		TRACE_FUNCTION();

		ionassertnd(m_AllocData.find(data) != m_AllocData.end());

		m_AllocData.erase(data);

		// @TODO: Defragment here
	}

	void AssetMemoryPool::Defragment()
	{

	}
}
