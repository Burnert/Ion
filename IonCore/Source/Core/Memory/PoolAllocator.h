#pragma once

#include "Core/Base.h"
#include "Core/Error/Error.h"
#include "MemoryCore.h"

#pragma warning(disable:6011)

#define ALLOCATOR __declspec(allocator)

#define POOL_META_ALLOC_FLAG_MASK 0x8000000000000000
#define POOL_META_POINTER_MASK    0x7FFFFFFFFFFFFFFF
#define GET_POOL_META_ALLOC_FLAG(meta)     ((meta) & POOL_META_ALLOC_FLAG_MASK)
#define GET_POOL_META_POINTER(meta) (void*)((meta) & POOL_META_POINTER_MASK)

namespace Ion
{
	template<typename T, size_t Alignment>
	struct alignas(Alignment) TPoolChunk
	{
		using ThisType = TPoolChunk<T, Alignment>;

		uint8 Data[sizeof(T)];
		/* The MS bit indicates if the chunk is allocated,
		   the rest is the pointer to the next chunk.
		   This is only possible, because pointers would not
		   normally use this bit as their value is too small. */
		uint64 Meta;

		inline void SetNext(ThisType* next)
		{
			Meta = GET_POOL_META_ALLOC_FLAG(Meta) | (uint64)next;
		}

		inline ThisType* Next() const
		{
			return (ThisType*)GET_POOL_META_POINTER(Meta);
		}

		inline bool IsAllocated() const
		{
			return GET_POOL_META_ALLOC_FLAG(Meta);
		}

		TPoolChunk() :
			Data(),
			Meta(0)
		{
		}
	};

	template<typename T, size_t ChunkCount, size_t Alignment>
	struct alignas(Alignment) TPoolBlock
	{
		using ThisType  = TPoolBlock<T, ChunkCount, Alignment>;
		using ChunkType = TPoolChunk<T, Alignment>;
		static constexpr size_t ChunksSize = sizeof(ChunkType) * ChunkCount;

		ChunkType Chunks[ChunkCount];
		ThisType* NextBlock;

		TPoolBlock() :
			NextBlock(nullptr),
			Chunks()
		{
			ChunkType* chunk = Chunks;
			// Don't set the last chunk's next
			for (size_t i = 0; i < ChunkCount - 1; ++i, ++chunk)
			{
				chunk->SetNext(chunk + 1);
			}
		}

		~TPoolBlock()
		{
			// Chain delete all the blocks
			checked_delete(NextBlock);
		}
	};

	template<typename T, size_t ChunksInBlock = 256, size_t Alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__>
	class TPoolAllocator
	{
	public:
		static_assert(ChunksInBlock != 0);

		using BlockType = TPoolBlock<T, ChunksInBlock, Alignment>;
		using ChunkType = TPoolChunk<T, Alignment>;

		NODISCARD inline ALLOCATOR T* Allocate()
		{
			// If the next chunk is null (there are no free chunks), we have to create a new block.
			if (!m_NextChunkPtr)
			{
				m_FreeBlockPtr = AllocateNewBlockAfter(m_FreeBlockPtr);
				m_NextChunkPtr = m_FreeBlockPtr->Chunks;
			}

			ChunkType* chunk = m_NextChunkPtr;
			chunk->Meta |= POOL_META_ALLOC_FLAG_MASK;

			m_NextChunkPtr = chunk->Next();
		
			return (T*)chunk->Data;
		}

		inline void Free(void* ptr)
		{
			BlockType* block = FindBlockByPtr(ptr);
			ionverify(block, "The pointer has not been allocated by this allocator.");

			ChunkType* chunk = (ChunkType*)ptr;
			ionverify(chunk->IsAllocated(), "The chunk has not been allocated.");

			chunk->Meta = (uint64)GET_POOL_META_POINTER((uint64)m_NextChunkPtr);
		
			m_FreeBlockPtr = block;
			m_NextChunkPtr = chunk;
		}

		TPoolAllocator()
		{
			m_FirstBlock = AllocateBlock();
			m_FreeBlockPtr = m_FirstBlock;
			m_NextChunkPtr = m_FreeBlockPtr->Chunks;
		}

		~TPoolAllocator()
		{
			delete m_FirstBlock;
		}

	private:
		inline BlockType* AllocateNewBlockAfter(BlockType* block)
		{
			ionassert(block);

			BlockType* newBlock = AllocateBlock();
			// Link the blocks.
			block->NextBlock = newBlock;

			return newBlock;
		}

		inline BlockType* FindBlockByPtr(void* ptr)
		{
			BlockType* block = m_FirstBlock;
			while (block && !IsPtrInBlockBounds((uint8*)ptr, block))
			{
				block = block->NextBlock;
			}
			return block;
		}

		inline static bool IsPtrInBlockBounds(uint8* ptr, BlockType* block)
		{
			return ptr >= (uint8*)block && ptr < (uint8*)block + BlockType::ChunksSize;
		}

		inline static BlockType* AllocateBlock()
		{
			return new BlockType;
		}

	private:
		BlockType* m_FirstBlock;
		BlockType* m_FreeBlockPtr;
		ChunkType* m_NextChunkPtr;
	};

	//template<typename T, size_t ChunksInBlock = 256>
	//class pool_allocator_wrapper
	//{
	//public:
	//
	//
	//private:
	//	TPoolAllocator<T, ChunksInBlock> m_PoolAllocator;
	//};

	// Force deferred compilation with the template
	template<void* = 0>
	inline void TestPoolAllocator()
	{
		struct A
		{
			char Text[32];
		};
		A* allocs[10] = {0};
		TPoolAllocator<A, 4> pool;
		for (int32 i = 0; i < 10; ++i)
		{
			allocs[i] = pool.Allocate();
			sprintf_s(allocs[i]->Text, "%d_Chunk_%d", i, i);
		}
		for (int32 i = 0; i < 10; ++i)
		{
			MemoryLogger.Trace("{0} - {1:5} - {2}", allocs[i]->Text, BoolStr(GET_POOL_META_ALLOC_FLAG(*(uint64*)(allocs[i] + 1))), (void*)allocs[i]);
		}
		pool.Free(allocs[0]);
		pool.Free(allocs[2]);
		pool.Free(allocs[5]);
		pool.Free(allocs[1]);
		pool.Free(allocs[9]);
		pool.Free(allocs[8]);
		pool.Free(allocs[4]);
		pool.Free(allocs[3]);
		pool.Free(allocs[7]);
		pool.Free(allocs[6]);
		MemoryLogger.Trace("--------------------------------------");
		for (int32 i = 0; i < 10; ++i)
		{
			MemoryLogger.Trace("{0} - {1:5} - {2}", allocs[i]->Text, BoolStr(GET_POOL_META_ALLOC_FLAG(*(uint64*)(allocs[i] + 1))), (void*)allocs[i]);
		}
		//debugbreak();
		for (int32 i = 0; i < 10; ++i)
		{
			allocs[i] = pool.Allocate();
			sprintf_s(allocs[i]->Text, "%d_Chunk_%d", i, i);
		}
		MemoryLogger.Trace("--------------------------------------");
		for (int32 i = 0; i < 10; ++i)
		{
			MemoryLogger.Trace("{0} - {1:5} - {2}", allocs[i]->Text, BoolStr(GET_POOL_META_ALLOC_FLAG(*(uint64*)(allocs[i] + 1))), (void*)allocs[i]);
		}
		//debugbreak();

		MemoryLogger.Info("TPoolAllocator:");
		for (int32 j = 0; j < 10; ++j)
		{
			TPoolAllocator<A, 256> pool2;
			size_t test = 0;
			size_t nAllocs = 65536;
			Ion::DebugTimer timer;
			for (int32 i = 0; i < nAllocs; ++i)
			{
				test += (uint64)pool2.Allocate();
			}
			timer.Stop();
			(void)test;
			MemoryLogger.Info("{0} allocations - {1:.6f}ms", nAllocs, timer.GetTime(EDebugTimerTimeUnit::Millisecond));
		}
		MemoryLogger.Info("Malloc:");
		for (int32 j = 0; j < 10; ++j)
		{
			size_t test = 0;
			size_t nAllocs = 65536;
			DebugTimer timer;
			for (int32 i = 0; i < nAllocs; ++i)
			{
				test += (uint64)malloc(sizeof(A));
			}
			timer.Stop();
			(void)test;
			MemoryLogger.Info("{0} allocations - {1:.6f}ms", nAllocs, timer.GetTime(EDebugTimerTimeUnit::Millisecond));
		}
	}
}
