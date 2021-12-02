#pragma once

#include "Core/File/File.h"

#define INVALID_ASSET_HANDLE_ID TNumericLimits<uint64>::max()
#define INVALID_ASSET_HANDLE AssetHandle(INVALID_ASSET_HANDLE_ID)

#define AMB_STRUCT alignas(16) // Asset message buffer struct specifier
#define ASSET_MESSAGE_TYPES \
AssetMessageBuffer, OnAssetLoadedMessage

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
	// Asset Handle ------------------------------------------------------------------
	// -------------------------------------------------------------------------------

	class ION_API AssetHandle
	{
	public:
		AssetHandle() :
			ID(INVALID_ASSET_HANDLE_ID)
		{ }

		bool IsValid() const;

		bool operator==(AssetHandle& other) const
		{
			return ID == other.ID;
		}

		bool operator!=(AssetHandle& other) const
		{
			return ID != other.ID;
		}

		// @TODO: Somehow don't let the user change the fields
		// of AssetReference but let them call non-const functions

		// Returns an AssetReference this handle points to by calling AssetManager
		AssetReference& GetRef();
		// Returns an AssetReference this handle points to by calling AssetManager
		const AssetReference& GetRef() const;

		// Returns an AssetReference this handle points to by calling AssetManager
		AssetReference* operator->()
		{
			return &GetRef();
		}
		// Returns an AssetReference this handle points to by calling AssetManager
		const AssetReference* operator->() const
		{
			return &GetRef();
		}

	private:
		AssetID ID;
		constexpr explicit AssetHandle(AssetID id) :
			ID(id)
		{ }

		friend class AssetManager;
	};

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
	}

	template<EAssetType Type>
	struct TAssetDescType { };
	template<>
	struct TAssetDescType<EAssetType::Mesh>    { using DescType = AssetTypes::MeshDesc; };
	template<>
	struct TAssetDescType<EAssetType::Texture> { using DescType = AssetTypes::TextureDesc; };

	template<EAssetType Type>
	using TAssetDescTypeT = typename TAssetDescType<Type>::DescType;

	// ------------------------------------------------------------------------------------------
	// Asset Messages / Events ------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------

	enum class EAssetMessageType : uint8
	{
		Null = 0,
		OnAssetLoaded
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

	using OnAssetLoadedEvent = TFunction<void(const OnAssetLoadedMessage&)>;

	struct AssetEvents
	{
		OnAssetLoadedEvent OnAssetLoaded;
	};

	// -------------------------------------------------------------------------------------------
	// Asset Reference ---------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------

	struct AssetData
	{
		void* Ptr;
		uint64 Size;
	};

	class AssetReference
	{
	public:
		AssetID ID;

		FilePath Location;

		AssetData Data;

		AssetEvents Events;

		void* Description;
		EAssetType Type;

		template<EAssetType Type>
		const TAssetDescTypeT<Type>* GetDescription() const
		{
			return (const TAssetDescTypeT<Type>*)Description;
		}

		template<EAssetType Type>
		TAssetDescTypeT<Type>* GetDescription()
		{
			return (TAssetDescTypeT<Type>*)Description;
		}

		template<typename Lambda>
		void LoadAssetData(Lambda callback)
		{
			AssetManager::Get()->LoadAssetData(*this, callback);
		}

		bool IsLoaded() const
		{
			return Data.Ptr;
		}

		AssetReference(const AssetReference&) = delete;
		AssetReference& operator=(const AssetReference&) = delete;

		AssetReference(AssetReference&&) noexcept = default;
		AssetReference& operator=(AssetReference&&) noexcept = default;

	private:
		friend class AssetManager;
		friend class AssetWorker;
	};
}
