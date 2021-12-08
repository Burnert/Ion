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

		UniqueLock lock(m_MessageQueueMutex);

		while (!m_MessageQueue.empty())
		{
			AssetMessageBuffer buffer = m_MessageQueue.top();
			m_MessageQueue.pop();

			lock.unlock();

			switch (buffer.Type)
			{
				_HANDLE_MESSAGE_CASE(OnAssetLoaded);
				_HANDLE_MESSAGE_CASE(OnAssetLoadError);
				_HANDLE_MESSAGE_CASE(OnAssetAllocError);
				_HANDLE_MESSAGE_CASE(OnAssetUnloaded);
				_HANDLE_MESSAGE_CASE(OnAssetRealloc);
			}

			lock.lock();
		}
	}

	template<typename T>
	inline void AssetManager::ScheduleAssetWorkerWork(T& work)
	{
		{
			UniqueLock lock(m_WorkQueueMutex);
			m_WorkQueue.emplace(MakeShared<T>(work));
		}
		m_WorkQueueCV.notify_one();
	}

	template<typename T>
	inline void AssetManager::AddMessage(T& message)
	{
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
		AssetManager::Get()->name(message); \
		checked_call(message.RefPtr->Events.name, message); \
	}

	template<typename T>
	inline static void AssetManager::_DispatchMessage(T& message)
	{
		_DISPATCH_MESSAGE_SELF(OnAssetLoaded);
		_DISPATCH_MESSAGE_SELF(OnAssetAllocError);
		_DISPATCH_MESSAGE_SELF(OnAssetUnloaded);
		_DISPATCH_MESSAGE_SELF(OnAssetRealloc);
		_DISPATCH_MESSAGE(OnAssetLoadError);
	}

	template<EAssetType Type>
	void AssetManager::DefragmentAssetPool()
	{
		TRACE_FUNCTION();

		AssetMemoryPool& poolRef = This_GetAssetPoolFromType(Type);

		poolRef.DefragmentPool([this](void* oldLocation, void* newLocation)
		{ // For each asset reallocation
			HandleAssetRealloc(oldLocation, newLocation);
		});
	}

	template<EAssetType Type>
	inline size_t AssetManager::GetAssetAlignment() const
	{
		const AssetMemoryPool& poolRef = This_GetAssetPoolFromType(Type);
		return poolRef.m_Alignment;
	}

	template<EAssetType Type>
	inline TShared<AssetMemoryPoolDebugInfo> AssetManager::GetAssetPoolDebugInfo() const
	{
		TRACE_FUNCTION();

		const AssetMemoryPool& poolRef = This_GetAssetPoolFromType(Type);

		return poolRef.GetDebugInfo();
	}

	template<EAssetType Type>
	inline void AssetManager::PrintAssetPool() const
	{
		TRACE_FUNCTION();

		const AssetMemoryPool& poolRef = This_GetAssetPoolFromType(Type);

		LOG_INFO("Asset Memory - {0} Pool Data:", AssetTypeToString(Type));

		poolRef.PrintDebugInfo();
	}

	// AssetWorker --------------------------------------------------------------------------------

	template<EAssetType Type>
	inline void* AssetWorker::AllocateAssetData(size_t size, AllocError_Details& outErrorData)
	{
		TRACE_FUNCTION();

		AssetMemoryPool& poolRef = GetAssetPoolFromType(m_Owner, Type);

		if (size > poolRef.GetSize())
		{
			LOG_WARN("Tried to allocate an asset larger than the entire memory pool.\n"
			"Pool Size = {0} B | Asset Size = {1} B\n"
			"If there is a need for large assets, try allocating a bigger pool beforehand.", poolRef.GetSize(), size);

			outErrorData.FailedAllocSize = size;
			outErrorData.bPoolOutOfMemory = true;
			outErrorData.bAllocSizeGreaterThanPoolSize = true;
			outErrorData.bPoolFragmented = poolRef.IsFragmented();
			return nullptr;
		}

		if (size > poolRef.GetFreeBytes())
		{
			outErrorData.FailedAllocSize = size;
			outErrorData.bPoolOutOfMemory = true;
			outErrorData.bPoolFragmented = poolRef.IsFragmented();
			return nullptr;
		}

		void* ptr = poolRef.Alloc(size);
		if (ptr)
			return ptr;

		if (poolRef.IsFragmented())
		{
			outErrorData.FailedAllocSize = size;
			outErrorData.bPoolFragmented = true;
			return nullptr;
		}

		outErrorData.FailedAllocSize = size;
		outErrorData.bOtherAllocError = true;
		return nullptr;
	}
}
