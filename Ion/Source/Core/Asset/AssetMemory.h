#pragma once

#define DEFAULT_ASSET_ALIGNMENT 64 // Cache line size for concurrent use

namespace Ion
{
	struct AssetAllocData
	{
		void* Ptr;
		size_t Size;
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
		void Defragment();

	private:
		AssetMemoryPool();

		THashMap<void*, AssetAllocData> m_AllocData;
		TArray<AssetAllocData> m_SequentialAllocData;

		void* m_Data;
		size_t m_Size;
		size_t m_Alignment;

		size_t m_NextOffset;

		size_t m_UsedBytes;

		mutable Mutex m_PoolMutex;

		friend class AssetManager;
	};
}
