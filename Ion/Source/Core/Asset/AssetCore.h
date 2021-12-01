#pragma once

#include "Core/File/File.h"

namespace Ion
{
#define INVALID_ASSET_HANDLE AssetHandle(TNumericLimits<uint64>::max())

	// Fwd decl
	class AssetManager;
	class AssetWorker;

	// @TODO: Make this a GUID
	using AssetID = uint64;
	extern AssetID g_CurrentAssetID;
	AssetID CreateAssetID();
	
	struct ION_API AssetHandle
	{
		bool IsValid() const;

		bool operator==(AssetHandle& other) const
		{
			return ID == other.ID;
		}

		bool operator!=(AssetHandle& other) const
		{
			return ID != other.ID;
		}

		// Returns an AssetReference this handle points to by calling AssetManager
		const struct AssetReference* operator->() const;

	private:
		AssetID ID;
		constexpr explicit AssetHandle(AssetID id) :
			ID(id)
		{ }

		friend class AssetManager;
	};

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
		struct TextureDesc
		{
			uint32 Width;
			uint32 Height;
			uint8 NumChannels;
			uint8 BytesPerChannel;
		};
	}

	struct AssetData
	{
		void* Ptr;
		uint64 Size;
	};

	struct AssetReference
	{
		AssetID ID;

		FilePath Location;

		AssetData Data;

		void* Description;
		EAssetType Type;

		template<typename T>
		const T* GetDescription() const
		{
			ionassert(dynamic_cast<T>(Description), "Invalid type cast.");
			return (T*)Description;
		}

		AssetReference(const AssetReference&) = delete;
		AssetReference& operator=(const AssetReference&) = delete;

		AssetReference(AssetReference&&) noexcept = default;
		AssetReference& operator=(AssetReference&&) = default;
	};
}
