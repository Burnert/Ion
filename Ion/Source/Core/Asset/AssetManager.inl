namespace Ion
{
	template<>
	inline void* AssetManager::AllocateAssetData<EAssetType::Texture>(size_t size)
	{
		TRACE_FUNCTION();

		return m_TextureAssetPool.Alloc(size);
	}
}
