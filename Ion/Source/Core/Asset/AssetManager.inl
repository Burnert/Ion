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

#define _DISPATCH_MESSAGE_CASE(type) \
	case EAssetMessageType::##type: \
		forEach(*(type##Message*)&buffer); \
		break

	template<typename ForEach>
	inline void AssetManager::IterateMessages(ForEach forEach)
	{
		TRACE_FUNCTION();

		while (!m_MessageQueue.empty())
		{
			UniqueLock lock(m_MessageQueueMutex);

			AssetMessageBuffer& buffer = m_MessageQueue.front();
			m_MessageQueue.pop();

			lock.unlock();

			switch (buffer.Type)
			{
				_DISPATCH_MESSAGE_CASE(OnAssetLoaded);
				_DISPATCH_MESSAGE_CASE(OnAssetLoadError);
				_DISPATCH_MESSAGE_CASE(OnAssetUnloaded);
				_DISPATCH_MESSAGE_CASE(OnAssetRealloc);
			}
		}
	}

	template<typename T>
	inline void AssetManager::AddMessage(T& message)
	{
		static_assert(TIsAnyOfV<T, TAssetMessageTypes>);

		UniqueLock lock(m_MessageQueueMutex);
		m_MessageQueue.emplace(Move((AssetMessageBuffer&)message));
	}

	template<EAssetType Type>
	void AssetManager::DefragmentAssetPool()
	{
		TRACE_FUNCTION();

		AssetMemoryPool& poolRef = _GetAssetPoolFromType(Type);

		poolRef.DefragmentPool([this](void* oldLocation, void* newLocation)
		{ // For each asset reallocation
			OnAssetReallocMessage message { };
			message.OldPoolLocation = oldLocation;
			message.NewPoolLocation = newLocation;
			message.RefPtr = m_LoadedAssets.at(oldLocation);

			_DispatchMessage(message);
		});
	}

	template<EAssetType Type>
	inline TShared<AssetMemoryPoolDebugInfo> AssetManager::GetAssetPoolDebugInfo() const
	{
		TRACE_FUNCTION();

		const AssetMemoryPool& poolRef = _GetAssetPoolFromType(Type);

		return poolRef.GetDebugInfo();
	}

	template<EAssetType Type>
	struct _TAssetPoolNameFromType;
	template<> struct _TAssetPoolNameFromType<EAssetType::Text>    { static constexpr const char* Name = "Text"; };
	template<> struct _TAssetPoolNameFromType<EAssetType::Mesh>    { static constexpr const char* Name = "Mesh"; };
	template<> struct _TAssetPoolNameFromType<EAssetType::Texture> { static constexpr const char* Name = "Texture"; };
	template<> struct _TAssetPoolNameFromType<EAssetType::Sound>   { static constexpr const char* Name = "Sound"; };

	template<EAssetType Type>
	inline void AssetManager::PrintAssetPool() const
	{
		TRACE_FUNCTION();

		const AssetMemoryPool& poolRef = _GetAssetPoolFromType(Type);

		LOG_INFO("Asset Memory - {0} Pool Data:", _TAssetPoolNameFromType<Type>::Name);

		poolRef.PrintDebugInfo();
	}

	// AssetManager::_DispatchMessage specializations -----------------------------------------------

	template<typename T>
	inline static void AssetManager::_DispatchMessage(T& message) { }

	template<>
	inline static void AssetManager::_DispatchMessage(OnAssetLoadedMessage& message)
	{
		AssetManager::Get()->OnAssetLoaded(message);

		checked_call(message.RefPtr->Events.OnAssetLoaded, message);
	}

	template<>
	inline static void AssetManager::_DispatchMessage(OnAssetLoadErrorMessage& message)
	{
		checked_call(message.RefPtr->Events.OnAssetLoadError, message);
	}

	template<>
	inline static void AssetManager::_DispatchMessage(OnAssetUnloadedMessage& message)
	{
		AssetManager::Get()->OnAssetUnloaded(message);

		checked_call(message.RefPtr->Events.OnAssetUnloaded, message);
	}

	template<>
	inline static void AssetManager::_DispatchMessage(OnAssetReallocMessage& message)
	{
		AssetManager::Get()->OnAssetRealloc(message);

		checked_call(message.RefPtr->Events.OnAssetRealloc, message);
	}

	// AssetManager::AllocateAssetData specializations -----------------------------------------------

	template<EAssetType Type>
	inline void* AssetManager::AllocateAssetData(size_t size) { }

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

	// AssetManager::GetAssetAlignment specializations -----------------------------------------------

	template<EAssetType Type>
	inline size_t AssetManager::GetAssetAlignment() const { }

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
