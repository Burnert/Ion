#pragma once

#include "Core/CoreTypes.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
	REGISTER_LOGGER(MemoryLogger, "Core::Memory");

	template<typename T>
	struct TMemoryBlock
	{
		T* Ptr;
		size_t Count;

		inline T* End() const
		{
			return Ptr + Count;
		}

		inline size_t Size() const
		{
			return Count * sizeof(T);
		}

		inline void Free()
		{
			if (Ptr)
				delete[] Ptr;
		}
	};

	template<>
	struct TMemoryBlock<void>
	{
		void* Ptr;
		size_t Size;

		inline void* End() const
		{
			return (uint8*)Ptr + Size;
		}

		inline void Free()
		{
			if (Ptr)
				delete[] Ptr;
		}
	};

	// Generic Memory Block
	using MemoryBlock = TMemoryBlock<void>;

	#define MEMORYBLOCK_FIELD_NAMED(blockName, ptrName, sizeName) \
	union { \
		MemoryBlock blockName; \
		struct { \
			void* ptrName; \
			size_t sizeName; \
		}; \
	}
	#define MEMORYBLOCK_FIELD MEMORYBLOCK_FIELD_NAMED(Block, Ptr, Size);

	#define TYPED_MEMORYBLOCK_FIELD_NAMED(T, blockName, ptrName, countName) \
	union { \
		TMemoryBlock<T> blockName; \
		struct { \
			T* ptrName; \
			size_t countName; \
		}; \
	}
	#define TYPED_MEMORYBLOCK_FIELD(T) TYPED_MEMORYBLOCK_FIELD_NAMED(T, Block, Ptr, Count);

	namespace Memory
	{
		struct AllocError_Details
		{
			size_t FailedAllocSize;
			union
			{
				uint8 Flags;
				struct
				{
					uint8 bPoolOutOfMemory : 1;
					uint8 bPoolFragmented : 1;
					uint8 bAllocSizeGreaterThanPoolSize : 1;
					uint8 bOtherAllocError : 1;
				};
			};
		};
	}
}
