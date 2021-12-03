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

		return AssetHandle(assetRef.ID);
	}

	template<typename Lambda>
	inline void AssetManager::LoadAssetData(AssetReference& ref, Lambda callback)
	{
		ref.Events.OnAssetLoaded = callback;

		if (!ref.Data.Ptr)
		{
			ScheduleAssetLoadWork(ref);
		}
		else
		{
			OnAssetLoadedMessage message { };
			message.RefPtr = &ref;
			ref.Events.OnAssetLoaded(message);
		}
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
	inline void AssetManager::AddMessage(T* message)
	{
		static_assert(TIsAnyOfV<T, ASSET_MESSAGE_TYPES>);
		{
			UniqueLock lock(m_MessageQueueMutex);
			m_MessageQueue.push(*(AssetMessageBuffer*)message);
		}
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
	inline size_t AssetManager::GetAssetAlignment<EAssetType::Mesh>()
	{
		return m_MeshAssetPool.m_Alignment;
	}

	template<>
	inline size_t AssetManager::GetAssetAlignment<EAssetType::Texture>()
	{
		return m_TextureAssetPool.m_Alignment;
	}
}
