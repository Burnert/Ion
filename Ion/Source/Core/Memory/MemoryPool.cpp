#include "IonPCH.h"

#include "MemoryPool.h"

namespace Ion
{
	MemoryPool::MemoryPool() :
		m_PoolBlock({ 0 }),
		m_Alignment(0),
		m_CurrentPtr(nullptr),
		m_UsedBytes(0)
	{
	}

	void MemoryPool::AllocPool(size_t size, size_t alignment)
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

	void MemoryPool::FreePool()
	{
		TRACE_FUNCTION();

		ionassert(m_Data);

		_aligned_free(m_Data);
		m_Data = nullptr;
		m_CurrentPtr = nullptr;
		m_Size = 0;
		m_Alignment = 0;
		m_UsedBytes = 0;
	}

	void* MemoryPool::Alloc(size_t size)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(size);

		Memory::AllocError_Details& errorDetails = *(Memory::AllocError_Details*)m_ErrorDetails;

		// Round up to m_Alignment
		size_t alignedSize = AlignAs(size, m_Alignment);

		if (alignedSize > m_Size)
		{
			m_LastErrorType = EMemoryPoolError::AllocError;

			MemoryLogger.Warn("Tried to allocate a block larger than the entire memory pool.\n"
				"Pool Size = {0} B | Block Size (after alignment) = {1} B\n"
				"If there is a need for large blocks, try allocating a bigger pool beforehand.", m_Size, size);

			errorDetails.FailedAllocSize = size;
			errorDetails.bPoolOutOfMemory = true;
			errorDetails.bAllocSizeGreaterThanPoolSize = true;
			errorDetails.bPoolFragmented = _IsFragmented();
			return nullptr;
		}

		UniqueLock lock(m_PoolMutex);

		if (alignedSize > m_Size - m_UsedBytes)
		{
			m_LastErrorType = EMemoryPoolError::AllocError;

			errorDetails.FailedAllocSize = size;
			errorDetails.bPoolOutOfMemory = true;
			errorDetails.bPoolFragmented = _IsFragmented();
			return nullptr;
		}

		uint8* allocPtr = m_CurrentPtr;

		if (allocPtr + alignedSize > m_PoolBlock.End())
		{
			m_LastErrorType = EMemoryPoolError::AllocError;

			errorDetails.FailedAllocSize = size;
			// The pool must be fragmented, if the block should fit but it doesn't.
			errorDetails.bPoolFragmented = true;
			return nullptr;
		}

		m_CurrentPtr += alignedSize;
		m_UsedBytes += alignedSize;

		// Save the sub-block
		MemoryPoolAllocData allocData { };
		allocData.Ptr = allocPtr;
		allocData.Size = alignedSize;
		allocData.SequentialIndex = m_AllocData.size();

		m_AllocDataByPtr.emplace(allocPtr, allocData.SequentialIndex);
		m_AllocData.emplace_back(Move(allocData));

		return allocPtr;
	}

	void MemoryPool::Free(void* data)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(data);

		UniqueLock lock(m_PoolMutex);

		auto& it = m_AllocDataByPtr.find(data);
		ionverify(it != m_AllocDataByPtr.end());

		size_t deleteIndex = it->second;

		MemoryPoolAllocData& allocData = m_AllocData[deleteIndex];
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

	bool MemoryPool::_IsFragmented() const
	{
		TRACE_FUNCTION();

		ionassert(m_Data);

		void* expectedPtr = m_Data;
		for (const MemoryPoolAllocData& allocData : m_AllocData)
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

	bool MemoryPool::IsFragmented() const
	{
		TRACE_FUNCTION();

		UniqueLock lock(m_PoolMutex);
		return _IsFragmented();
	}

	bool MemoryPool::CanAlloc(size_t size) const
	{
		UniqueLock lock(m_PoolMutex);
		return AlignAs(size, m_Alignment) <= GetFreeBytes();
	}

	TShared<MemoryPoolDebugInfo> MemoryPool::GetDebugInfo() const
	{
		TRACE_FUNCTION();

		TShared<MemoryPoolDebugInfo> info = MakeShared<MemoryPoolDebugInfo>();

		UniqueLock lock(m_PoolMutex);

		info->PoolPtr = m_Data;
		info->PoolSize = m_Size;
		info->AllocCount = m_AllocData.size();
		info->BytesFree = GetFreeBytes();
		info->BytesUsed = GetUsedBytes();
		info->AllocData = m_AllocData;
		return info;
	}

	void MemoryPool::PrintDebugInfo() const
	{
		TRACE_FUNCTION();

		TShared<MemoryPoolDebugInfo> info = GetDebugInfo();

		MemoryLogger.Debug("Used: {0} B ({1:.2f} MB) | Free: {2} B ({3:.2f} MB) | Alloc count: {4}",
			info->BytesUsed, info->BytesUsed / (float)(1 << 20),
			info->BytesFree, info->BytesFree / (float)(1 << 20),
			info->AllocCount);
		MemoryLogger.Debug("----------------------------------------------------------------------------------");

		void* expectedPtr = info->PoolPtr;

		for (const MemoryPoolAllocData& allocData : info->AllocData)
		{
			if (allocData.Ptr != expectedPtr)
			{
				MemoryLogger.Error("Noncontiguous memory from [{0}] to [{1}]", expectedPtr, allocData.Ptr);
				size_t size = (uint8*)allocData.Ptr - (uint8*)expectedPtr;
				MemoryLogger.Error("Size: {0} B ({1:.2f} MB)", size, size / (float)(1 << 20));
			}
			expectedPtr = (uint8*)allocData.Ptr + allocData.Size;

			MemoryLogger.Debug("{0:<8d} | Ptr: [{1}] | Size: {2:20d} B ({3:.2f} MB)",
				allocData.SequentialIndex, allocData.Ptr, allocData.Size, allocData.Size / (float)(1 << 20));
		}
		MemoryLogger.Debug("----------------------------------------------------------------------------------");
	}
}
