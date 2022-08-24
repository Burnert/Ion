#pragma once

#include "MemoryCore.h"
#include "Core/Diagnostics/Tracing.h"

namespace Ion
{
	struct MemoryPoolAllocData
	{
		MEMORYBLOCK_FIELD;
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

	struct MemoryPoolDebugInfo
	{
		void* PoolPtr;
		size_t PoolSize;
		size_t BytesUsed;
		size_t BytesFree;
		size_t AllocCount;
		TArray<MemoryPoolAllocData> AllocData;
	};

	using OnBlockReallocCallback = TFunction<void(void* oldPtr, void* newPtr)>;

	enum class EMemoryPoolError : uint8
	{
		Null = 0,
		AllocError
	};

	class ION_API MemoryPool
	{
	public:
		using MemoryPoolErrorTypes = TTypePack<Memory::AllocError_Details>;

		using AllocDataArray = TArray<MemoryPoolAllocData>;
		using AllocDataByPtrMap = THashMap<void*, size_t>;

		MemoryPool();

		void AllocPool(size_t size, size_t alignment);
		void FreePool();
		template<typename Lambda>
		void ReallocPool(size_t newSize, Lambda onBlockRealloc);
		template<typename Lambda>
		void DefragmentPool(Lambda onBlockRealloc);

		void* Alloc(size_t size);
		void Free(void* data);

		bool IsFragmented() const;
		bool CanAlloc(size_t size) const;

		size_t GetSize() const { return m_Size; }
		size_t GetAlignment() const { return m_Alignment; }

		size_t GetUsedBytes() const { return m_UsedBytes; }
		size_t GetFreeBytes() const { return m_Size - m_UsedBytes; }

		EMemoryPoolError GetLastError() const { return m_LastErrorType; }
		template<typename T>
		T GetErrorDetails() const;

		std::shared_ptr<MemoryPoolDebugInfo> GetDebugInfo() const;
		void PrintDebugInfo() const;

		MemoryPool(const MemoryPool&) = delete;
		MemoryPool(MemoryPool&&) noexcept = delete;
		MemoryPool& operator=(const MemoryPool&) = delete;
		MemoryPool& operator=(MemoryPool&&) noexcept = delete;

		~MemoryPool() { }

	private:
		inline size_t GetCurrentOffset() const
		{
			return m_CurrentPtr - (uint8*)m_Data;
		}

		template<typename Lambda>
		void UpdateAllocData(ptrdiff_t offset, AllocDataArray::iterator start, AllocDataArray::iterator end, Lambda onBlockRealloc);

		bool _IsFragmented() const; // Non-locking

	private:
		AllocDataArray m_AllocData;
		AllocDataByPtrMap m_AllocDataByPtr;

		MEMORYBLOCK_FIELD_NAMED(m_PoolBlock, m_Data, m_Size);
		uint8* m_CurrentPtr;

		size_t m_Alignment;
		size_t m_UsedBytes;

		mutable Mutex m_PoolMutex;

		uint8 m_ErrorDetails[TTypeSize<MemoryPoolErrorTypes>::Max];
		EMemoryPoolError m_LastErrorType;
	};
}
#include "MemoryPool.inl"
