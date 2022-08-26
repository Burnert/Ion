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
		static TResourceRef<TextureResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the Texture owned by the Resource.
		 * 
		 * @details Use the GetRenderData function to retrieve the loaded objects.
		 * 
		 * @tparam Lambda params - (const TResourceRef<TextureResource>&)
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

		/**
		 * @brief Parses the TextureResource node in the .iasset file.
		 * Called by Resource::Query
		 *
		 * @see Resource::Query
		 *
		 * @param asset Asset handle
		 * @param outGuid GUID object to write the resource Guid to.
		 * @param outDescription TextureResourceDescription object to write to
		 * @return True if the file has been parsed successfully.
		 */
		static bool ParseAssetFile(const Asset& asset, GUID& outGuid, TextureResourceDescription& outDescription);

		DEFINE_RESOURCE_AS_REF(TextureResource)

	protected:
		TextureResource(const Asset& asset, const TextureResourceDescription& desc) :
			Resource(asset),
			m_RenderData({ }),
			m_Description(desc)
		{
		}

	private:
		TextureResourceRenderData m_RenderData;
		TextureResourceDescription m_Description;

		friend class Resource;
	};

	template<typename Lambda>
	inline bool TextureResource::Take(Lambda onTake)
	{
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<TextureResource>>);

		ionassert(m_Asset);

		TResourceRef<TextureResource> self = AsRef();

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
				ionassert(m_Asset->GetType() == EAssetType::Image);

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
