#pragma once

#include "AssetCommon.h"

namespace Ion
{
#pragma region Asset Type abstract base class

	class IAssetType;

	class IAssetCustomData
	{
	public:
		virtual IAssetType& GetType() const = 0;
	};

	// Implemented in AssetRegistry.cpp
	ION_API IAssetType& _RegisterAssetType(std::unique_ptr<IAssetType>&& customAssetType);

#define REGISTER_ASSET_TYPE_CLASS(T) inline T& AT_##T = static_cast<T&>(_RegisterAssetType(std::make_unique<T>()))
#define ASSET_TYPE_NAME_IMPL(name) \
	virtual const String& GetName() const override { \
		static String c_Name = name; \
		return c_Name; \
	}
#define ASSET_TYPE_DEFAULT_DATA_INL_IMPL(TType, TData) \
	inline TSharedPtr<IAssetCustomData> TType::CreateDefaultCustomData() const { \
		return MakeShared<TData>(); \
	}
#define ASSET_DATA_GETTYPE_IMPL(type) \
	virtual IAssetType& GetType() const override { \
		return type; \
	}

	class IAssetType
	{
	public:
		/**
		 * @brief Serialize the custom asset data relevant for the type.
		 * 
		 * @param ar Archive reference to (de)serialize to/from.
		 * @param inOutCustomData Custom asset data that will be (de)serialized to/from the archive.
		 * 
		 * @details This function is called after the generic asset definition data is serialized,
		 * which means the XML archive will already be inside an <IonAsset> node.
		 */
		virtual Result<void, IOError> Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const { ionthrow(IOError, "Serialize function not implemented."); }

		virtual TSharedPtr<IAssetCustomData> CreateDefaultCustomData() const = 0;

		virtual const String& GetName() const = 0;

		bool operator==(const IAssetType& other) const;
		bool operator!=(const IAssetType& other) const;

		size_t GetHash() const;
	};

	FORCEINLINE bool IAssetType::operator==(const IAssetType& other) const
	{
		return GetName() == other.GetName();
	}

	FORCEINLINE bool IAssetType::operator!=(const IAssetType& other) const
	{
		return GetName() != other.GetName();
	}

	FORCEINLINE size_t IAssetType::GetHash() const
	{
		return THash<String>()(GetName());
	}

#pragma endregion
}
