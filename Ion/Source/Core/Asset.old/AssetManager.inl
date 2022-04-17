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

#define _HANDLE_MESSAGE_CASE(type) \
	case EAssetMessageType::##type: \
		forEach(*(type##Message*)&buffer); \
		break

	template<typename ForEach>
	inline void AssetManager::IterateMessages(ForEach forEach)
	{
		TRACE_FUNCTION();

		MessageQueue messages;
		{
			// Retrieve the messages first, so new ones can be added
			// while the current ones are processing
			UniqueLock lock(m_MessageQueueMutex);
			m_MessageQueue.swap(messages);
		}

		while (!messages.empty())
		{
			TRACE_SCOPE("AssetManager::IterateMessages - Iterate Message");

			AssetMessageBuffer buffer = messages.top();

			messages.pop();

			switch (buffer.Type)
			{
				_HANDLE_MESSAGE_CASE(OnAssetLoaded);
				_HANDLE_MESSAGE_CASE(OnAssetLoadError);
				_HANDLE_MESSAGE_CASE(OnAssetAllocError);
				_HANDLE_MESSAGE_CASE(OnAssetUnloaded);
				_HANDLE_MESSAGE_CASE(OnAssetRealloc);
			}
		}
	}

	template<typename T>
	inline void AssetManager::ScheduleAssetWorkerWork(T& work)
	{
		TRACE_FUNCTION();

		static_assert(TIsBaseOfV<AssetWorkerWorkBase, T>);
		{
			UniqueLock lock(m_WorkQueueMutex);
			m_WorkQueue.emplace(MakeShared<T>(Move(work)));
		}
		m_WorkQueueCV.notify_one();
	}

	template<typename T>
	inline void AssetManager::AddMessage(T& message)
	{
		TRACE_FUNCTION();

		static_assert(TIsAnyOfV<T, TAssetMessageTypes>);

		UniqueLock lock(m_MessageQueueMutex);
		m_MessageQueue.emplace(Move((AssetMessageBuffer&)message));
	}

#define _DISPATCH_MESSAGE(name) \
	if constexpr (TIsSameV<T, name##Message>) { \
		checked_call(message.RefPtr->Events.name, message); \
	}
#define _DISPATCH_MESSAGE_SELF(name) \
	if constexpr (TIsSameV<T, name##Message>) { \
		AssetManager::name(message); \
		checked_call(message.RefPtr->Events.name, message); \
	}

	template<typename T>
	inline static void AssetManager::_DispatchMessage(T& message)
	{
		TRACE_FUNCTION();

		_DISPATCH_MESSAGE_SELF(OnAssetLoaded);
		_DISPATCH_MESSAGE     (OnAssetLoadError);
		_DISPATCH_MESSAGE_SELF(OnAssetAllocError);
		_DISPATCH_MESSAGE_SELF(OnAssetUnloaded);
		_DISPATCH_MESSAGE_SELF(OnAssetRealloc);
	}

	template<EAssetType Type>
	inline void AssetManager::DefragmentAssetPool()
	{
		TRACE_FUNCTION();

		MemoryPool& poolRef = GetAssetMemoryPool<Type>();

		poolRef.DefragmentPool([](void* oldLocation, void* newLocation)
		{ // For each asset reallocation
			HandleAssetRealloc(oldLocation, newLocation);
		});
	}

	template<EAssetType Type>
	MemoryPool& AssetManager::GetAssetMemoryPool()
	{
		if constexpr (Type == EAssetType::Mesh)
			return GetMeshAssetMemoryPool();
		if constexpr (Type == EAssetType::Texture)
			return GetTextureAssetMemoryPool();
	}

	template<EAssetType Type>
	inline size_t AssetManager::GetAssetAlignment()
	{
		const MemoryPool& poolRef = GetAssetMemoryPool<Type>();
		return poolRef.GetAlignment();
	}

	template<EAssetType Type>
	inline TShared<MemoryPoolDebugInfo> AssetManager::GetAssetPoolDebugInfo()
	{
		TRACE_FUNCTION();

		const MemoryPool& poolRef = GetAssetMemoryPool<Type>();

		return poolRef.GetDebugInfo();
	}

	template<EAssetType Type>
	inline void AssetManager::PrintAssetPool()
	{
		TRACE_FUNCTION();

		const MemoryPool& poolRef = GetAssetMemoryPool<Type>();

		LOG_INFO("Asset Memory - {0} Pool Data:", AssetTypeToString(Type));

		poolRef.PrintDebugInfo();
	}

	// AssetWorker --------------------------------------------------------------------------------

	template<EAssetType Type>
	inline void* AssetWorker::AllocateAssetData(size_t size, Memory::AllocError_Details& outErrorData)
	{
		TRACE_FUNCTION();

		MemoryPool& poolRef = AssetManager::TAssetPoolFromType<Type>;

		if (void* ptr = poolRef.Alloc(size))
			return ptr;

		EMemoryPoolError error = poolRef.GetLastError();
		if (error == EMemoryPoolError::AllocError)
		{
			outErrorData = poolRef.GetErrorDetails<Memory::AllocError_Details>();
		}

		return nullptr;
	}
}
