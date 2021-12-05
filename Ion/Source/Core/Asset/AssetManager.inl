namespace Ion
{
	template<EAssetType Type>
	inline AssetHandle AssetManager::CreateAsset(FilePath location)
	{
		using DescType = TAssetDescTypeT<Type>;

		TRACE_FUNCTION();

		ionassert(location.Exists());

		AssetReference asset { };
		asset.ID = CreateAssetID();
		asset.Location = location;
		asset.Type = Type;

		AssetReference& assetRef = m_Assets.emplace(asset.ID, Move(asset)).first->second;

		return AssetHandle(assetRef.ID, &assetRef);
	}

	template<typename ForEach>
	inline void AssetManager::IterateMessages(ForEach forEach)
	{
		TRACE_FUNCTION();

		UniqueLock lock(m_MessageQueueMutex);

		while (!m_MessageQueue.empty())
		{
			AssetMessageBuffer& buffer = m_MessageQueue.front();
			m_MessageQueue.pop();

			lock.unlock();

			switch (buffer.Type)
			{
				case EAssetMessageType::OnAssetLoaded:
				{
					forEach(*(OnAssetLoadedMessage*)&buffer);
					break;
				}
			}

			lock.lock();
		}
	}

	template<typename T>
	inline void AssetManager::AddMessage(T& message)
	{
		static_assert(TIsAnyOfV<T, TAssetMessageTypes>);
		{
			UniqueLock lock(m_MessageQueueMutex);
			m_MessageQueue.emplace(Move((AssetMessageBuffer&)message));
		}
	}

	template<typename T>
	inline static void AssetManager::_DispatchMessage(T& message) { }
	
	template<>
	inline static void AssetManager::_DispatchMessage(OnAssetLoadedMessage& message)
	{
		checked_call(message.RefPtr->Events.OnAssetLoaded, message);
	}

	template<>
	inline static void AssetManager::_DispatchMessage(OnAssetLoadErrorMessage& message)
	{
		checked_call(message.RefPtr->Events.OnAssetLoadError, message);
	}

	// AssetManager::AllocateAssetData -----------------------------------------------

	template<>
	inline void* AssetManager::AllocateAssetData<EAssetType::Mesh>(size_t size)
	{
		TRACE_FUNCTION();

		return m_MeshAssetPool.Alloc(size);
	}

	template<>
	inline void* AssetManager::AllocateAssetData<EAssetType::Texture>(size_t size)
	{
		TRACE_FUNCTION();

		return m_TextureAssetPool.Alloc(size);
	}

	// AssetManager::GetAssetAlignment -----------------------------------------------

	template<>
	inline size_t AssetManager::GetAssetAlignment<EAssetType::Mesh>() const
	{
		return m_MeshAssetPool.m_Alignment;
	}

	template<>
	inline size_t AssetManager::GetAssetAlignment<EAssetType::Texture>() const
	{
		return m_TextureAssetPool.m_Alignment;
	}
}
