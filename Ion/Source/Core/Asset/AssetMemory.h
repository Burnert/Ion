#pragma once

#define DEFAULT_ASSET_ALIGNMENT 64

namespace Ion
{
	struct AssetAllocData
	{
		void* Ptr;
		size_t Size;
	};

	class ION_API AssetMemoryPool
	{
	public:
		AssetMemoryPool(const AssetMemoryPool&) = delete;
		AssetMemoryPool& operator=(const AssetMemoryPool&) = delete;

		~AssetMemoryPool() { }

		void AllocatePool(size_t size, size_t alignment);
		void FreePool();
		void ReallocPool(size_t newSize);

		void* Alloc(size_t size);
		void Free(void* data);

	private:
		void Defragment();

	private:
		AssetMemoryPool();

		THashMap<void*, AssetAllocData> m_AllocData;
		size_t m_Size;
		size_t m_Alignment;
		size_t m_NextOffset;
		void* m_Data;

		friend class AssetManager;
	};
}
