#pragma once

#include "Core/CoreTypes.h"
#include "Core/Diagnostics/Tracing.h"

#define DEFAULT_ASSET_ALIGNMENT 64 // Cache line size for concurrent use

namespace Ion
{
	struct MemoryBlockDescriptor
	{
		void* Ptr;
		size_t Size;

		void* End() const
		{
			return (uint8*)Ptr + Size;
		}
	};

#define MEMORY_BLOCK_FIELD \
	union { \
		MemoryBlockDescriptor Block; \
		struct { \
			void* Ptr; \
			size_t Size; \
		}; \
	}

	struct AssetAllocData
	{
		MEMORY_BLOCK_FIELD;
		size_t SequentialIndex;
	};

	struct AssetMemoryPoolDebugInfo
	{
		void* PoolPtr;
		size_t PoolSize;
		size_t BytesUsed;
		size_t BytesFree;
		size_t AllocCount;
		TArray<AssetAllocData> AllocData;
	};

	class ION_API AssetMemoryPool
	{
	public:
		void AllocatePool(size_t size, size_t alignment);
		void FreePool();
		void ReallocPool(size_t newSize);
		template<typename Lambda>
		void DefragmentPool(Lambda onItemRealloc);

		void* Alloc(size_t size);
		void Free(void* data);

		size_t GetUsedBytes() const { return m_UsedBytes; }
		size_t GetFreeBytes() const { return m_Size - m_UsedBytes; }

		TShared<AssetMemoryPoolDebugInfo> GetDebugInfo() const;
		void PrintDebugInfo() const;

		AssetMemoryPool(const AssetMemoryPool&) = delete;
		AssetMemoryPool(AssetMemoryPool&&) noexcept = delete;
		AssetMemoryPool& operator=(const AssetMemoryPool&) = delete;
		AssetMemoryPool& operator=(AssetMemoryPool&&) noexcept = delete;

		~AssetMemoryPool() { }

	private:
		inline size_t GetCurrentOffset() const
		{
			return m_CurrentPtr - m_Data;
		}

	private:
		AssetMemoryPool();

		THashMap<void*, AssetAllocData> m_AllocData;
		TArray<AssetAllocData> m_SequentialAllocData;

		union
		{
			MemoryBlockDescriptor m_PoolBlock;
			struct
			{
				uint8* m_Data;
				size_t m_Size;
			};
		};
		size_t m_Alignment;

		uint8* m_CurrentPtr;

		size_t m_UsedBytes;

		mutable Mutex m_PoolMutex;

		friend class AssetManager;
	};

	template<typename Lambda>
	inline void AssetMemoryPool::DefragmentPool(Lambda onItemRealloc)
	{
		TRACE_FUNCTION();

		struct _NonContiguousMemoryBlock
		{
			MemoryBlockDescriptor Block;
			AssetAllocData* LeftAllocDataPtr;
			AssetAllocData* RightAllocDataPtr;
		};

		if (m_AllocData.empty())
			return;

		TDeque<_NonContiguousMemoryBlock> nonContiguousBlocks;

		// Find non-contiguous memory blocks
		void* expectedPtr = m_Data;
		AssetAllocData* previousAllocData = nullptr;
		for (AssetAllocData& allocData : m_SequentialAllocData)
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

			AssetAllocData* right = blocksIt->RightAllocDataPtr;

			uint8* destination = (uint8*)blocksIt->Block.Ptr;
			uint8* source = (uint8*)right->Ptr;

			size_t blocksSize = 0;
			// Find out how many bytes to move
			// (Could be shorter but this is more readable)
			if (!bNext)
			{ // If there's no another NC memory, move all the blocks
				AssetAllocData& lastAllocData = m_SequentialAllocData.back();
				uint8* endOfChunk = (uint8*)lastAllocData.Block.End();
				blocksSize = endOfChunk - source;
			}
			else
			{ // Move all the blocks before the next NC memory
				uint8* nextNCMem = (uint8*)nextIt->Block.Ptr;
				blocksSize = nextNCMem - source;
			}

			// Move the alloc blocks to the start of the noncontiguous memory
			memmove(destination, source, blocksSize);

			ptrdiff_t offsetAfterMove = destination - source;

			// Update the alloc data
			for (auto allocDataIt = m_SequentialAllocData.begin() + right->SequentialIndex;
				allocDataIt != (bNext ? (
					m_SequentialAllocData.begin() + nextIt->LeftAllocDataPtr->SequentialIndex + 1) :
					m_SequentialAllocData.end());
				++allocDataIt)
			{
				uint8* oldPtr = (uint8*)allocDataIt->Ptr;
				uint8* movedPtr = (uint8*)allocDataIt->Ptr + offsetAfterMove;

				auto allocNode = m_AllocData.extract(allocDataIt->Ptr);
				allocNode.mapped().Ptr = movedPtr;
				allocNode.key() = movedPtr;
				m_AllocData.insert(Move(allocNode));

				allocDataIt->Ptr = movedPtr;

				onItemRealloc(oldPtr, movedPtr);
			}

			if (nextIt != nonContiguousBlocks.end())
			{
				// Shift and join the noncontiguous block
				nextIt->Block.Ptr = destination + blocksSize;
				nextIt->Block.Size += blocksIt->Block.Size;
				nonContiguousBlocks.erase(blocksIt);
			}

			blocksIt = nextIt;
		}
		// Update the next alloc pointer after all of that
		m_CurrentPtr = (uint8*)m_SequentialAllocData.back().Block.End();
	}
}
