#pragma once

#include "Core/File/File.h"
#include "Core/Memory/MemoryCore.h"

namespace Ion
{
	// Fwd decl
	class AssetManager;
	class AssetWorker;
	class AssetReference;
	class VertexLayout;

	// @TODO: Make this a GUID
	using AssetID = uint64;
	extern AssetID g_CurrentAssetID;
	AssetID CreateAssetID();

	// -------------------------------------------------------------------------------
	// Asset Types -------------------------------------------------------------------
	// -------------------------------------------------------------------------------

	enum class EAssetType : uint8
	{
		Null = 0,
		Text,
		Mesh,
		Texture,
		Sound
	};

	FORCEINLINE constexpr const char* AssetTypeToString(EAssetType type)
	{
		switch (type)
		{
			case EAssetType::Text:    return "Text";
			case EAssetType::Mesh:    return "Mesh";
			case EAssetType::Texture: return "Texture";
			case EAssetType::Sound:   return "Sound";
		}
		return "";
	}

	namespace AssetDescription
	{
		struct Text
		{

		};

		struct Mesh
		{
			uint64 VerticesOffset;
			uint64 IndicesOffset;
			uint64 VertexCount;
			uint64 IndexCount;
			TShared<VertexLayout> VertexLayout;
		};

		struct Texture
		{
			uint32 Width;
			uint32 Height;
			uint8 NumChannels;
			uint8 BytesPerChannel;
		};

		struct Sound
		{

		};

		using TAssetDescriptionTypes = TTypePack<
			Text,
			Mesh,
			Texture,
			Sound
		>;

		inline constexpr size_t LargestDescSize = TTypeSize<TAssetDescriptionTypes>::Max;

		struct DescBuffer
		{
			uint8 _Buf[LargestDescSize] = { 0 };
		};
	}

	template<EAssetType V>
	struct TAssetDescType;
	template<>
	struct TAssetDescType<EAssetType::Text>    { using Type = AssetDescription::Text; };
	template<>
	struct TAssetDescType<EAssetType::Mesh>    { using Type = AssetDescription::Mesh; };
	template<>
	struct TAssetDescType<EAssetType::Texture> { using Type = AssetDescription::Texture; };
	template<>
	struct TAssetDescType<EAssetType::Sound>   { using Type = AssetDescription::Sound; };

	template<EAssetType V>
	using TAssetDescTypeT = typename TAssetDescType<V>::Type;

	// ------------------------------------------------------------------------------------------
	// Asset Messages / Events ------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// When adding new message / event, there are a few things to take care of
	// 1. Add a new message to this section based on what is already in here
	// 2. Handle message dispatch in the message queue
	//    - Add _HANDLE_MESSAGE_CASE() macro to AssetManager::IterateMessages in AssetManager.inl
	//    - Add _DISPATCH_MESSAGE() or _DISPATCH_MESSAGE_SELF() macro to AssetManager::_DispatchMessage in AssetManager.inl
	// 3. Add _ASSIGN_MSG_EVENT_CALLBACK() macro to AssetInterface::AssignEvent below in this file
	// ------------------------------------------------------------------------------------------

#define ASSET_MESSAGE_BUFFER alignas(8) // Asset message buffer struct specifier

	enum class EAssetMessageType : uint8
	{ // Messages with a lower index will be handled first
		Null = 0,
		OnAssetLoaded,
		OnAssetLoadError,
		OnAssetAllocError,
		OnAssetUnloaded,
		OnAssetRealloc,
	};

	struct ASSET_MESSAGE_BUFFER OnAssetLoadedMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetLoaded;
		AssetReference* RefPtr;
		void* PoolLocation;
	};

	struct ASSET_MESSAGE_BUFFER OnAssetLoadErrorMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetLoadError;
		AssetReference* RefPtr;
		const char* ErrorMessage;
	};

	struct ASSET_MESSAGE_BUFFER OnAssetAllocErrorMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetAllocError;
		EAssetType AssetType;
		AssetReference* RefPtr;
		Memory::AllocError_Details ErrorDetails;
	};

	struct ASSET_MESSAGE_BUFFER OnAssetUnloadedMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetUnloaded;
		AssetReference* RefPtr;
		void* LastPoolLocation;
	};

	struct ASSET_MESSAGE_BUFFER OnAssetReallocMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetRealloc;
		AssetReference* RefPtr;
		void* OldPoolLocation;
		void* NewPoolLocation;
	};

	using TAssetMessageTypes = TTypePack<
		OnAssetLoadedMessage,
		OnAssetLoadErrorMessage,
		OnAssetAllocErrorMessage,
		OnAssetUnloadedMessage,
		OnAssetReallocMessage
	>;

	struct ASSET_MESSAGE_BUFFER AssetMessageBuffer
	{
		EAssetMessageType Type;
	private:
		uint8 _Padding[TTypeSize<TAssetMessageTypes>::Max - 1];

	public:
		friend bool operator>(const AssetMessageBuffer& left, const AssetMessageBuffer& right)
		{
			return left.Type > right.Type;
		}
	};

	using OnAssetLoadedEvent     = TFunction<void(const OnAssetLoadedMessage&)>;
	using OnAssetLoadErrorEvent  = TFunction<void(const OnAssetLoadErrorMessage&)>;
	using OnAssetAllocErrorEvent = TFunction<void(const OnAssetAllocErrorMessage&)>;
	using OnAssetUnloadedEvent   = TFunction<void(const OnAssetUnloadedMessage&)>;
	using OnAssetReallocEvent    = TFunction<void(const OnAssetReallocMessage&)>;

	struct AssetEvents
	{
		OnAssetLoadedEvent OnAssetLoaded;
		OnAssetLoadErrorEvent OnAssetLoadError;
		OnAssetAllocErrorEvent OnAssetAllocError;
		OnAssetUnloadedEvent OnAssetUnloaded;
		OnAssetReallocEvent OnAssetRealloc;
	};

	// -------------------------------------------------------------------------------------------
	// Asset Reference ---------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------

	struct AssetData
	{
		MEMORYBLOCK_FIELD;
	};

	using AssetReferenceErrorDataTypes = TTypePack<
		Memory::AllocError_Details
	>;

	class ION_API AssetReference
	{
	public:
		AssetID ID;
		FilePath Location;
		AssetData Data;
		AssetEvents Events;
		AssetDescription::DescBuffer Description;
		uint8 ErrorData[TTypeSize<AssetReferenceErrorDataTypes>::Max];
		EAssetType Type;
		union
		{
			uint8 PackedFlags;
			struct
			{
				uint8 bScheduledLoad : 1;
				uint8 bAllocError : 1;
			};
		};

		template<EAssetType Type>
		inline const TAssetDescTypeT<Type>* GetDescription() const
		{
			return (const TAssetDescTypeT<Type>*)Description._Buf;
		}

		template<EAssetType Type>
		inline TAssetDescTypeT<Type>* GetDescription()
		{
			return (TAssetDescTypeT<Type>*)Description._Buf;
		}

		inline bool IsLoaded() const
		{
			return (bool)Data.Ptr;
		}

		inline bool IsLoading() const
		{
			return bScheduledLoad;
		}

		AssetReference(const AssetReference&) = delete;
		AssetReference& operator=(const AssetReference&) = delete;

		AssetReference(AssetReference&&) noexcept = default;
		AssetReference& operator=(AssetReference&&) noexcept = default;

	private:
		AssetReference();

		void SetErrorData(const MemoryBlock& data);

		friend class AssetManager;
		friend class AssetWorker;
	};

	// -------------------------------------------------------------------------------
	// Asset Interface ---------------------------------------------------------------
	// -------------------------------------------------------------------------------

	class ION_API AssetInterface
	{
	public:
		template<typename Lambda>
		void AssignEvent(Lambda callback);

		void LoadAssetData();
		void UnloadAssetData();

		inline bool IsLoaded() const
		{
			return (bool)m_RefPtr->IsLoaded();
		}

		inline bool IsLoading() const
		{
			return (bool)m_RefPtr->IsLoading();
		}

		inline EAssetType GetType() const
		{
			return m_RefPtr->Type;
		}

		inline void* Data() const
		{
			return m_RefPtr->Data.Ptr;
		}

		inline size_t DataSize() const
		{
			return m_RefPtr->Data.Size;
		}

		template<EAssetType Type>
		inline const TAssetDescTypeT<Type>* GetDescription() const
		{
			return m_RefPtr->GetDescription<Type>();
		}

		template<EAssetType Type>
		inline TAssetDescTypeT<Type>* GetDescription()
		{
			return m_RefPtr->GetDescription<Type>();
		}

	private:
		AssetInterface() = delete;
		constexpr explicit AssetInterface(AssetReference* refPtr) :
			m_RefPtr(refPtr)
		{ }

		AssetReference* m_RefPtr;

		friend class AssetHandle;
		friend class AssetManager;
	};

	// For external use ---------------------------------------

#define _ASSIGN_ASSET_MSG_EVENT_CALLBACK(e) \
	if constexpr (TIsConvertibleV<Lambda, e##Event>) \
		m_RefPtr->Events.e = callback

	template<typename Lambda>
	inline void AssetInterface::AssignEvent(Lambda callback)
	{
		_ASSIGN_ASSET_MSG_EVENT_CALLBACK(OnAssetLoaded);
		_ASSIGN_ASSET_MSG_EVENT_CALLBACK(OnAssetLoadError);
		_ASSIGN_ASSET_MSG_EVENT_CALLBACK(OnAssetAllocError);
		_ASSIGN_ASSET_MSG_EVENT_CALLBACK(OnAssetUnloaded);
		_ASSIGN_ASSET_MSG_EVENT_CALLBACK(OnAssetRealloc);
	}

	// -------------------------------------------------------------------------------
	// Asset Handle ------------------------------------------------------------------
	// -------------------------------------------------------------------------------

#define INVALID_ASSET_ID TNumericLimits<uint64>::max()
#define INVALID_ASSET_HANDLE AssetHandle(INVALID_ASSET_ID, nullptr)

	class ION_API AssetHandle
	{
	public:
		AssetHandle() :
			m_ID(INVALID_ASSET_ID),
			m_Interface(AssetInterface(nullptr))
		{
		}

		bool IsValid() const;

		inline bool operator==(AssetHandle& other) const
		{
			return m_ID == other.m_ID;
		}

		inline bool operator!=(AssetHandle& other) const
		{
			return m_ID != other.m_ID;
		}

		// Returns an AssetInterface used to communicate with AssetManager through the handle
		inline AssetInterface& GetInterface()
		{
			ionassert(IsValid(), "Tried to access interface of invalid handle.");
			return m_Interface;
		}
		// Returns an AssetInterface used to communicate with AssetManager through the handle
		inline const AssetInterface& GetInterface() const
		{
			ionassert(IsValid(), "Tried to access interface of invalid handle.");
			return m_Interface;
		}

		// Returns an AssetInterface used to communicate with AssetManager through the handle
		inline AssetInterface* operator->()
		{
			return &GetInterface();
		}
		// Returns an AssetInterface used to communicate with AssetManager through the handle
		inline const AssetInterface* operator->() const
		{
			return &GetInterface();
		}

	private:
		AssetID m_ID;
		AssetInterface m_Interface;
		constexpr explicit AssetHandle(AssetID id, AssetReference* refPtr) :
			m_ID(id),
			m_Interface(AssetInterface(refPtr))
		{
		}

		friend class AssetManager;
	};
}
