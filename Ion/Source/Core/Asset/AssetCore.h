#pragma once

#include "Core/File/File.h"

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

	namespace AssetTypes
	{
		struct TextDesc
		{

		};

		struct MeshDesc
		{
			uint64 VerticesOffset;
			uint64 IndicesOffset;
			uint64 VertexCount;
			uint64 IndexCount;
			TShared<VertexLayout> VertexLayout;
		};

		struct TextureDesc
		{
			uint32 Width;
			uint32 Height;
			uint8 NumChannels;
			uint8 BytesPerChannel;
		};

		struct SoundDesc
		{

		};

		static constexpr size_t LargestDescSize = std::max({
			sizeof(MeshDesc),
			sizeof(TextureDesc)
		});
	}

	template<EAssetType Type>
	struct TAssetDescType { };
	template<>
	struct TAssetDescType<EAssetType::Text>    { using DescType = AssetTypes::TextDesc; };
	template<>
	struct TAssetDescType<EAssetType::Mesh>    { using DescType = AssetTypes::MeshDesc; };
	template<>
	struct TAssetDescType<EAssetType::Texture> { using DescType = AssetTypes::TextureDesc; };
	template<>
	struct TAssetDescType<EAssetType::Sound>   { using DescType = AssetTypes::SoundDesc; };

	template<EAssetType Type>
	using TAssetDescTypeT = typename TAssetDescType<Type>::DescType;

	// ------------------------------------------------------------------------------------------
	// Asset Messages / Events ------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------

#define AMB_STRUCT alignas(16) // Asset message buffer struct specifier

	enum class EAssetMessageType : uint8
	{
		Null = 0,
		OnAssetLoaded,
		OnAssetLoadError,
	};

	struct AMB_STRUCT AssetMessageBuffer
	{
		EAssetMessageType Type;
	private:
		uint8 _Padding[31];
	};

	struct AMB_STRUCT OnAssetLoadedMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetLoaded;
		AssetReference* RefPtr;
	};

	struct AMB_STRUCT OnAssetLoadErrorMessage
	{
		EAssetMessageType Type = EAssetMessageType::OnAssetLoadError;
		AssetReference* RefPtr;
		const char* ErrorMessage;
	};

#define ASSET_MESSAGE_TYPES \
	AssetMessageBuffer, OnAssetLoadedMessage, OnAssetLoadErrorMessage

	using OnAssetLoadedEvent    = TFunction<void(const OnAssetLoadedMessage&)>;
	using OnAssetLoadErrorEvent = TFunction<void(const OnAssetLoadErrorMessage&)>;

	struct AssetEvents
	{
		OnAssetLoadedEvent OnAssetLoaded;
		OnAssetLoadErrorEvent OnAssetLoadError;
	};

	// -------------------------------------------------------------------------------------------
	// Asset Reference ---------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------

	struct AssetData
	{
		void* Ptr;
		uint64 Size;
	};

	class ION_API AssetReference
	{
	public:
		AssetID ID;

		FilePath Location;

		AssetData Data;

		AssetEvents Events;

		uint8 Description[AssetTypes::LargestDescSize];
		EAssetType Type;

		template<EAssetType Type>
		inline const TAssetDescTypeT<Type>* GetDescription() const
		{
			return (const TAssetDescTypeT<Type>*)Description;
		}

		template<EAssetType Type>
		inline TAssetDescTypeT<Type>* GetDescription()
		{
			return (TAssetDescTypeT<Type>*)Description;
		}

		inline bool IsLoaded() const
		{
			return (bool)Data.Ptr;
		}

		AssetReference(const AssetReference&) = delete;
		AssetReference& operator=(const AssetReference&) = delete;

		AssetReference(AssetReference&&) noexcept = default;
		AssetReference& operator=(AssetReference&&) noexcept = default;

	private:
		AssetReference();

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

		inline bool IsLoaded() const
		{
			return (bool)m_RefPtr->IsLoaded();
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

	template<typename Lambda>
	inline void AssetInterface::AssignEvent(Lambda callback)
	{
		if constexpr (TIsConvertibleV<Lambda, OnAssetLoadedEvent>)
		{
			m_RefPtr->Events.OnAssetLoaded = callback;
		}
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
		bool IsLoaded() const;

		inline EAssetType GetType() const
		{
			return (*this).GetInterface().GetType();
		}

		inline bool operator==(AssetHandle& other) const
		{
			return m_ID == other.m_ID;
		}

		inline bool operator!=(AssetHandle& other) const
		{
			return m_ID != other.m_ID;
		}

		// @TODO: Somehow don't let the user change the fields
		// of AssetReference but let them call non-const functions

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
