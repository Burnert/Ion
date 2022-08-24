namespace Ion
{
	template<typename Lambda>
	inline void MemoryPool::UpdateAllocData(ptrdiff_t offset, AllocDataArray::iterator start, AllocDataArray::iterator end, Lambda onBlockRealloc)
	{
		TRACE_FUNCTION();

		for (auto allocDataIt = start; allocDataIt != end; ++allocDataIt)
		{
			uint8* oldPtr = (uint8*)allocDataIt->Ptr;
			uint8* movedPtr = (uint8*)allocDataIt->Ptr + offset;

			allocDataIt->Ptr = movedPtr;

			auto allocNode = m_AllocDataByPtr.extract(oldPtr);
			allocNode.key() = movedPtr;
			m_AllocDataByPtr.insert(Move(allocNode));

			if constexpr (TIsConvertibleV<Lambda, OnBlockReallocCallback>)
				onBlockRealloc(oldPtr, movedPtr);
		}
	}

	template<typename Lambda>
	inline void MemoryPool::ReallocPool(size_t newSize, Lambda onBlockRealloc)
	{
		TRACE_FUNCTION();

		ionassert(m_Data);
		ionassert(newSize);

		UniqueLock lock(m_PoolMutex);

		// @TODO: FIX: Sometimes this newSize is equal to the current offset.
		ionassert(newSize > GetCurrentOffset());

		newSize = AlignAs(newSize, m_Alignment);

		void* data = _aligned_malloc(newSize, m_Alignment);
		size_t size = Math::Min(m_Size, newSize);
		memcpy(data, m_Data, size);

		ptrdiff_t offsetAfterMove = (uint8*)data - (uint8*)m_Data;

		_aligned_free(m_Data);
		m_Size = newSize;
		m_Data = data;
		m_CurrentPtr += offsetAfterMove;

		UpdateAllocData(offsetAfterMove, m_AllocData.begin(), m_AllocData.end(), onBlockRealloc);
	}

	template<typename Lambda>
	inline void MemoryPool::DefragmentPool(Lambda onBlockRealloc)
	{
		TRACE_FUNCTION();

		if (!IsFragmented())
			return;

		struct _NonContiguousMemoryBlock
		{
			MemoryBlock Block;
			MemoryPoolAllocData* LeftAllocDataPtr;
			MemoryPoolAllocData* RightAllocDataPtr;
		};

		if (m_AllocData.empty())
			return;

		TDeque<_NonContiguousMemoryBlock> nonContiguousBlocks;

		// Find non-contiguous memory blocks
		void* expectedPtr = m_Data;
		MemoryPoolAllocData* previousAllocData = nullptr;
		for (MemoryPoolAllocData& allocData : m_AllocData)
		{
			// Memory is non-contiguous if the next data block
			// does not start at the end of the previous one
			if (allocData.Ptr != expectedPtr)
			{
				_NonContiguousMemoryBlock ncBlock { };
				ncBlock.Block.Ptr = expectedPtr;
				ncBlock.Block.Size = (uint8*)allocData.Ptr - (uint8*)expectedPtr;

				ncBlock.LeftAllocDataPtr = previousAllocData;
				ncBlock.RightAllocDataPtr = &allocData;

				nonContiguousBlocks.emplace_back(Move(ncBlock));
			}
			expectedPtr = allocData.Block.End();
			previousAllocData = &allocData;
		}

		// Don't defragment if the memory is already fully contiguous
		if (nonContiguousBlocks.empty())
			return;

		auto blocksIt = nonContiguousBlocks.begin();
		while (blocksIt != nonContiguousBlocks.end())
		{
			auto nextIt = blocksIt + 1;
			bool bNext = nextIt != nonContiguousBlocks.end();

			MemoryPoolAllocData* right = blocksIt->RightAllocDataPtr;

			uint8* destination = (uint8*)blocksIt->Block.Ptr;
			uint8* source = (uint8*)right->Ptr;

			size_t blocksSize = 0;
			// Find out how many bytes to move
			// (Could be shorter but this is more readable)
			if (!bNext)
			{ // If there's no another NC memory, move all the blocks
				MemoryPoolAllocData& lastAllocData = m_AllocData.back();
				uint8* endOfChunk = (uint8*)lastAllocData.Block.End();
				blocksSize = endOfChunk - source;
			}
			else
			{ // Move all the blocks before the next NC memory
				uint8* nextNCMem = (uint8*)nextIt->Block.Ptr;
				blocksSize = nextNCMem - source;
			}

			// Move the alloc blocks to the start of the NC memory
			memmove(destination, source, blocksSize);

			ptrdiff_t offsetAfterMove = destination - source;

			// Update the alloc data
			UpdateAllocData(offsetAfterMove,
				m_AllocData.begin() + right->SequentialIndex,
				bNext ? (m_AllocData.begin() + nextIt->LeftAllocDataPtr->SequentialIndex + 1) : m_AllocData.end(),
				onBlockRealloc);

			if (nextIt != nonContiguousBlocks.end())
			{
				// Shift and join the NC block
				nextIt->Block.Ptr = destination + blocksSize;
				nextIt->Block.Size += blocksIt->Block.Size;
				nonContiguousBlocks.erase(blocksIt);
			}

			blocksIt = nextIt;
		}
		// Update the next alloc pointer after all of that
		m_CurrentPtr = (uint8*)m_AllocData.back().Block.End();
	}

	template<typename T>
	T MemoryPool::GetErrorDetails() const
	{
		return *(T*)m_ErrorDetails;
	}
}
