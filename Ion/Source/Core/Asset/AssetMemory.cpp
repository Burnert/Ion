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

		size = AlignAs(size, alignment);

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

	void* AssetMemoryPool::Alloc(size_t size)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(size);

		UniqueLock lock(m_PoolMutex);

		if (size > GetFreeBytes())
			return nullptr;

		uint8* allocPtr = m_CurrentPtr;

		// Round up to m_Alignment
		size_t alignedSize = AlignAs(size, m_Alignment);

		if (allocPtr + alignedSize > m_PoolBlock.End())
			return nullptr;

		m_CurrentPtr += alignedSize;
		m_UsedBytes += alignedSize;

		// Save the sub-block
		AssetAllocData allocData { };
		allocData.Ptr = allocPtr;
		allocData.Size = alignedSize;
		allocData.SequentialIndex = m_AllocData.size();

		m_AllocDataByPtr.emplace(allocPtr, allocData.SequentialIndex);
		m_AllocData.emplace_back(Move(allocData));

		return allocPtr;
	}

	void AssetMemoryPool::Free(void* data)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(data);

		UniqueLock lock(m_PoolMutex);

		auto& it = m_AllocDataByPtr.find(data);
		ionassertnd(it != m_AllocDataByPtr.end());

		size_t deleteIndex = it->second;

		AssetAllocData& allocData = m_AllocData[deleteIndex];
		m_UsedBytes -= allocData.Size;

		m_AllocData.erase(m_AllocData.begin() + deleteIndex);
		m_AllocDataByPtr.erase(it);

		// Fix indices
		for (auto seqIt = m_AllocData.begin() + deleteIndex; seqIt != m_AllocData.end(); ++seqIt)
		{
			seqIt->SequentialIndex--;
			m_AllocDataByPtr[seqIt->Ptr]--;
		}

		// Set the current pointer to the first free spot
		m_CurrentPtr = !m_AllocData.empty() ? (uint8*)m_AllocData.back().Block.End() : (uint8*)m_Data;
	}

	bool AssetMemoryPool::IsFragmented() const
	{
		TRACE_FUNCTION();

		ionassert(m_Data);

		UniqueLock lock(m_PoolMutex);

		void* expectedPtr = m_Data;
		for (const AssetAllocData& allocData : m_AllocData)
		{
			// Memory is non-contiguous if the next data block
			// does not start at the end of the previous one
			if (allocData.Ptr != expectedPtr)
			{
				return true;
			}
			expectedPtr = allocData.Block.End();
		}
		return false;
	}

	bool AssetMemoryPool::CanAlloc(size_t size) const
	{
		UniqueLock lock(m_PoolMutex);
		return size <= GetFreeBytes();
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
		info->AllocData = m_AllocData;
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
