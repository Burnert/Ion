#include "IonPCH.h"

#include "AssetMemory.h"

#pragma warning(disable:6387)

namespace Ion
{
	AssetMemoryPool::AssetMemoryPool() :
		m_Data(nullptr),
		m_Size(0),
		m_Alignment(0),
		m_NextOffset(0),
		m_UsedBytes(0)
	{ }

	void AssetMemoryPool::AllocatePool(size_t size, size_t alignment)
	{
		TRACE_FUNCTION();

		ionassert(!m_Data);
		ionassert(size);
		ionassert(Math::IsPowerOfTwo(alignment));

		m_Data = _aligned_malloc(size, alignment);
		m_Size = size;
		m_Alignment = alignment;
		m_NextOffset = 0;
	}

	void AssetMemoryPool::FreePool()
	{
		TRACE_FUNCTION();

		ionassert(m_Data);

		_aligned_free(m_Data);
		m_Data = nullptr;
		m_Size = 0;
		m_Alignment = 0;
		m_NextOffset = 0;
		m_UsedBytes = 0;
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
		m_Size = newSize;
		m_Data = data;
	}

	void* AssetMemoryPool::Alloc(size_t size)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(size);

		uint8* current;
		{
			UniqueLock lock(m_PoolMutex);

			ionassertnd(size <= GetFreeBytes());

			current = (uint8*)m_Data + m_NextOffset;

			// Round up to m_Alignment
			size_t alignedSize = AlignAs(size, m_Alignment);

			m_NextOffset += alignedSize;
			m_UsedBytes += alignedSize;

			// @TODO: Reallocate if the offset goes out of the buffer
			if (m_NextOffset > m_Size)
			{
				ionassertnd(false);
				return nullptr;
			}

			// Save the sub-block
			AssetAllocData allocData { };
			allocData.Ptr = current;
			allocData.Size = alignedSize;
			allocData.SequentialIndex = m_SequentialAllocData.size();

			m_AllocData.emplace(current, Move(allocData));
			m_SequentialAllocData.emplace_back(Move(allocData));
		}
		return current;
	}

	void AssetMemoryPool::Free(void* data)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(data);
		{
			UniqueLock lock(m_PoolMutex);

			auto& it = m_AllocData.find(data);
			ionassertnd(it != m_AllocData.end());

			AssetAllocData& allocData = (*it).second;

			m_UsedBytes -= allocData.Size;

			m_SequentialAllocData.erase(m_SequentialAllocData.begin() + allocData.SequentialIndex);

			// Fix indices
			for (auto it = m_SequentialAllocData.begin() + allocData.SequentialIndex; it != m_SequentialAllocData.end(); it++)
			{
				(*it).SequentialIndex--;
				m_AllocData[(*it).Ptr].SequentialIndex--;
			}

			m_AllocData.erase(data);

			Defragment();
		}
	}

	TShared<AssetMemoryPoolDebugInfo> AssetMemoryPool::GetDebugInfo() const
	{
		TRACE_FUNCTION();

		TShared<AssetMemoryPoolDebugInfo> info = MakeShared<AssetMemoryPoolDebugInfo>();

		UniqueLock lock(m_PoolMutex);

		info->PoolPtr = m_Data;
		info->PoolSize = m_Size;
		info->AllocCount = m_AllocData.size();
		info->BytesFree = GetFreeBytes();
		info->BytesUsed = GetUsedBytes();
		info->AllocData = m_SequentialAllocData;
		return info;
	}

	void AssetMemoryPool::PrintDebugInfo() const
	{
		TRACE_FUNCTION();

		TShared<AssetMemoryPoolDebugInfo> info = GetDebugInfo();

		LOG_DEBUG("Used: {0} B ({1:.2f} MB) | Free: {2} B ({3:.2f} MB) | Alloc count: {4}",
			info->BytesUsed, info->BytesUsed / (float)(1 << 20),
			info->BytesFree, info->BytesFree / (float)(1 << 20),
			info->AllocCount);
		LOG_DEBUG("----------------------------------------------------------------------------------");

		void* expectedPtr = info->PoolPtr;

		for (const AssetAllocData& allocData : info->AllocData)
		{
			if (allocData.Ptr != expectedPtr)
			{
				LOG_ERROR("Noncontiguous memory from [{0}] to [{1}]", expectedPtr, allocData.Ptr);
				size_t size = (uint8*)allocData.Ptr - (uint8*)expectedPtr;
				LOG_ERROR("Size: {0} B ({1:.2f} MB)", size, size / (float)(1 << 20));
			}
			expectedPtr = (uint8*)allocData.Ptr + allocData.Size;

			LOG_DEBUG("Index: {0:<8d} | Ptr: [{1}] | Size: {2:20d} B ({3:.2f} MB)",
				allocData.SequentialIndex, allocData.Ptr, allocData.Size, allocData.Size / (float)(1 << 20));
		}
		LOG_DEBUG("----------------------------------------------------------------------------------");
	}

	void AssetMemoryPool::Defragment()
	{
		TRACE_FUNCTION();

		for (AssetAllocData& allocData : m_SequentialAllocData)
		{

		}
	}
}
