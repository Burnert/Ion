#include "IonPCH.h"

#include "AssetMemory.h"

#pragma warning(disable:6387)

namespace Ion
{
	AssetMemoryPool::AssetMemoryPool() :
		m_PoolBlock({ 0 }),
		m_Alignment(0),
		m_CurrentPtr(nullptr),
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
		m_CurrentPtr = (uint8*)m_Data;
	}

	void AssetMemoryPool::FreePool()
	{
		TRACE_FUNCTION();

		ionassert(m_Data);

		_aligned_free(m_Data);
		m_Data = nullptr;
		m_Size = 0;
		m_Alignment = 0;
		m_CurrentPtr = nullptr;
		m_UsedBytes = 0;
	}

	void AssetMemoryPool::ReallocPool(size_t newSize)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(newSize);
		ionassert(newSize > GetCurrentOffset());

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

		UniqueLock lock(m_PoolMutex);

		ionassertnd(size <= GetFreeBytes());

		uint8* allocPtr = m_CurrentPtr;

		// Round up to m_Alignment
		size_t alignedSize = AlignAs(size, m_Alignment);

		m_CurrentPtr += alignedSize;
		m_UsedBytes += alignedSize;

		// @TODO: Reallocate if the offset goes out of the buffer
		if (m_CurrentPtr > m_PoolBlock.End())
		{
			ionassertnd(false);
			return nullptr;
		}

		// Save the sub-block
		AssetAllocData allocData { };
		allocData.Ptr = allocPtr;
		allocData.Size = alignedSize;
		allocData.SequentialIndex = m_SequentialAllocData.size();

		m_AllocData.emplace(allocPtr, Move(allocData));
		m_SequentialAllocData.emplace_back(Move(allocData));

		return allocPtr;
	}

	void AssetMemoryPool::Free(void* data)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(data);

		UniqueLock lock(m_PoolMutex);

		auto& it = m_AllocData.find(data);
		ionassertnd(it != m_AllocData.end());

		AssetAllocData& allocData = it->second;
		m_UsedBytes -= allocData.Size;
		size_t deleteIndex = allocData.SequentialIndex;

		m_SequentialAllocData.erase(m_SequentialAllocData.begin() + deleteIndex);
		m_AllocData.erase(it);

		// Fix indices
		for (auto seqIt = m_SequentialAllocData.begin() + deleteIndex; seqIt != m_SequentialAllocData.end(); ++seqIt)
		{
			seqIt->SequentialIndex--;
			m_AllocData[seqIt->Ptr].SequentialIndex--;
		}

		// Set the current pointer to the first free spot
		m_CurrentPtr = !m_SequentialAllocData.empty() ? (uint8*)m_SequentialAllocData.back().Block.End() : (uint8*)m_Data;
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

			LOG_DEBUG("{0:<8d} | Ptr: [{1}] | Size: {2:20d} B ({3:.2f} MB)",
				allocData.SequentialIndex, allocData.Ptr, allocData.Size, allocData.Size / (float)(1 << 20));
		}
		LOG_DEBUG("----------------------------------------------------------------------------------");
	}
}
