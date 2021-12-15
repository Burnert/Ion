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

#define MEMORY_BLOCK_FIELD_NAMED(blockName, ptrName, sizeName) \
	union { \
		MemoryBlockDescriptor blockName; \
		struct { \
			void* ptrName; \
			size_t sizeName; \
		}; \
	}
#define MEMORY_BLOCK_FIELD MEMORY_BLOCK_FIELD_NAMED(Block, Ptr, Size);

	struct AssetAllocData
	{
		MEMORY_BLOCK_FIELD;
		size_t SequentialIndex;
	};

#define MEMORY_ALLOC_ERROR_FLAGS \
	union { \
		uint8 Flags; \
		struct { \
			uint8 bPoolOutOfMemory : 1; \
			uint8 bPoolFragmented : 1; \
			uint8 bAllocSizeGreaterThanPoolSize : 1; \
			uint8 bOtherAllocError : 1; \
		}; \
	}

	struct AllocError_Details
	{
		size_t FailedAllocSize;
		MEMORY_ALLOC_ERROR_FLAGS;
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
		using AllocDataArray = TArray<AssetAllocData>;
		using AllocDataByPtrMap = THashMap<void*, size_t>;

		void AllocatePool(size_t size, size_t alignment);
		void FreePool();
		template<typename Lambda>
		void ReallocPool(size_t newSize, Lambda onItemRealloc);
		template<typename Lambda>
		void DefragmentPool(Lambda onItemRealloc);

		void* Alloc(size_t size);
		void Free(void* data);

		bool IsFragmented() const;

		bool CanAlloc(size_t size) const;

		size_t GetSize() const { return m_Size; }

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
			return m_CurrentPtr - (uint8*)m_Data;
		}

		template<typename Lambda>
		void UpdateAllocData(ptrdiff_t offset, AllocDataArray::iterator start, AllocDataArray::iterator end, Lambda onItemRealloc);

		AssetMemoryPool();

	private:
		AllocDataArray m_AllocData;
		AllocDataByPtrMap m_AllocDataByPtr;

		MEMORY_BLOCK_FIELD_NAMED(m_PoolBlock, m_Data, m_Size);
		uint8* m_CurrentPtr;

		size_t m_Alignment;
		size_t m_UsedBytes;

		mutable Mutex m_PoolMutex;

		friend class AssetManager;
	};

	using OnBlockReallocCallback = TFunction<void(void*, void*)>;
}
#include "AssetMemory.inl"
