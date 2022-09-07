#pragma once

#include "Resource.h"
#include "RHI/Texture.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Texture Resource
	// ------------------------------------------------------------

	/**
	 * @brief Texture Resource properties representation from the asset file.
	 */
	struct TextureResourceProperties
	{
		ETextureFilteringMethod Filter = ETextureFilteringMethod::Default;
	};

	/**
	 * @brief Resource description representation from the asset file.
	 */
	struct TextureResourceDescription
	{
		TextureResourceProperties Properties;
	};

	struct TextureResourceRenderData
	{
		TRef<RHITexture> Texture;

		bool IsAvailable() const
		{
			return (bool)Texture;
		}
	};

#pragma region Image Asset Type

	class ImageAssetType : public IAssetType
	{
	public:
		virtual Result<TSharedPtr<IAssetCustomData>, IOError> Parse(const std::shared_ptr<XMLDocument>& xml) const override;
		ASSET_TYPE_NAME_IMPL("Ion.Image")
	};

	REGISTER_ASSET_TYPE_CLASS(ImageAssetType);

	class ImageAssetData : public IAssetCustomData
	{
	public:
		virtual IAssetType& GetType() const override
		{
			return AT_ImageAssetType;
		}

		GUID ResourceGuid;
		TextureResourceDescription Description;
	};

#pragma endregion

	class ION_API TextureResource : public Resource
	{
	public:
		using TResourceDescription = TextureResourceDescription;

		/**
		 * @brief Query the Resource Manager for a Texture Resource
		 * associated with the specified asset.
		 * 
		 * @see Resource::Query(const Asset& asset)
		 * 
		 * @param asset Asset associated with the Resource
		 * @return Shared pointer to the Resource
		 */
		static TSharedPtr<TextureResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the Texture owned by the Resource.
		 * 
		 * @details Use the GetRenderData function to retrieve the loaded objects.
		 * 
		 * @tparam Lambda params - (const TSharedPtr<TextureResource>&)
		 * @see TFuncResourceOnTake
		 * 
		 * @param onTake If the resource is ready, called immediately,
		 * else, called as soon as the resource is loaded (on the main thread).
		 * 
		 * @return Returns true if the resource is available instantly.
		 */
		template<typename Lambda>
		bool Take(Lambda onTake);

		/**
		 * @brief Get the resource render data, even if it hasn't been loaded yet.
		 * 
		 * @return TextureResource render data
		 */
		const TextureResourceRenderData& GetRenderData() const;

		virtual bool IsLoaded() const override;

	protected:
		TextureResource(const Asset& asset) :
			Resource(asset),
			m_RenderData({ })
		{
			ionassert(asset->GetType() == AT_ImageAssetType);
			TSharedPtr<ImageAssetData> data = PtrCast<ImageAssetData>(asset->GetCustomData());
			m_Description = data->Description;
		}

	private:
		TextureResourceRenderData m_RenderData;
		TextureResourceDescription m_Description;

		friend class Resource;
		FRIEND_MAKE_SHARED;
	};

	template<typename Lambda>
	inline bool TextureResource::Take(Lambda onTake)
	{
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<TextureResource>>);

		ionassert(m_Asset);

		TSharedPtr<TextureResource> self = PtrCast<TextureResource>(SharedFromThis());

		if (m_RenderData.IsAvailable())
		{
			onTake(self);
			return true;
		}

		ResourceLogger.Trace("Texture Resource \"{}\" render data is unavailable.", m_Asset->GetVirtualPath());
		m_Asset->Import(
			[self](std::shared_ptr<AssetFileMemoryBlock> block)
			{
				ResourceLogger.Trace("Importing Texture Resource from Asset \"{}\"...", self->m_Asset->GetVirtualPath());
				return AssetImporter::ImportImageAsset(block);
			},
			// Store the ref (self) so the resource doesn't get deleted before it's loaded
			[this, self, onTake](std::shared_ptr<Image> image)
			{
				ResourceLogger.Info("Texture Resource from Asset \"{}\" has been imported successfully.", m_Asset->GetVirtualPath());

				ionassert(m_Asset);
				ionassert(m_Asset->GetType() == AT_ImageAssetType);

				TextureDescription desc { };
				desc.Dimensions.Width = image->GetWidth();
				desc.Dimensions.Height = image->GetHeight();
				desc.bGenerateMips = true;
				desc.bCreateSampler = true;
				desc.bUseAsRenderTarget = true;
				desc.DebugName = m_Asset->GetVirtualPath();

				desc.SetFilterAll(m_Description.Properties.Filter);

				m_RenderData.Texture = RHITexture::Create(desc);

				m_RenderData.Texture->UpdateSubresource(image.get());

				ResourceLogger.Trace("Texture Resource \"{}\" render data is now available.", m_Asset->GetVirtualPath());

				onTake(self);
			},
			[self](auto& result) { ResourceLogger.Error("Failed to import Texture Resource from Asset \"{}\". {}", self->m_Asset->GetVirtualPath(), result.GetErrorMessage()); }
		);
		return false;
	}

	inline const TextureResourceRenderData& TextureResource::GetRenderData() const
	{
		return m_RenderData;
	}
}
