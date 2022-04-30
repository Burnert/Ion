#pragma once

#include "Resource.h"
#include "RHI/Texture.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Texture Resource
	// ------------------------------------------------------------

	/**
	 * @brief Resource description representation from the asset file.
	 */
	struct TextureResourceDescription
	{
		ETextureFilteringMethod Filter;
	};

	struct TextureResourceRenderData
	{
		// @TODO: this shouldn't be shader ptr

		TShared<RHITexture> Texture;

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
		static TResourcePtr<TextureResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the Texture owned by the Resource.
		 * 
		 * @tparam Lambda params - (const TextureResourceRenderData&)
		 * @see TFuncResourceOnTake
		 * 
		 * @param onTake If the resource is ready, called immediately,
		 * else, called as soon as the resource is loaded.
		 * 
		 * @return Returns true if the resource is available instantly.
		 */
		template<typename Lambda>
		bool Take(Lambda onTake);

		virtual bool IsLoaded() const override;

		/**
		 * @brief Parses the TextureResource node in the .iasset file.
		 * Called by Resource::Query
		 *
		 * @see Resource::Query
		 *
		 * @param path .iasset file path
		 * @param outGuid GUID object to write the resource Guid to.
		 * @param outDescription TextureResourceDescription object to write to
		 * @return True if the file has been parsed successfully.
		 */
		static bool ParseAssetFile(const FilePath& path, GUID& outGuid, TextureResourceDescription& outDescription);

	protected:
		TextureResource(const GUID& guid, const Asset& asset, const TextureResourceDescription& desc) :
			Resource(guid, asset),
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
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<TextureResourceRenderData>>);

		if (m_RenderData.IsAvailable())
		{
			onTake(m_RenderData);
			return true;
		}

		auto initTexture = [this, onTake](const AssetData& data)
		{
			ionassert(m_Asset->GetType() == EAssetType::Image);

			TShared<Image> image = data.Get<Image>();

			TextureDescription desc { };
			desc.Dimensions.Width = image->GetWidth();
			desc.Dimensions.Height = image->GetHeight();
			desc.bGenerateMips = true;
			desc.bCreateSampler = true;
			desc.bUseAsRenderTarget = true;
			desc.DebugName = StringConverter::WStringToString(m_Asset->GetDefinitionPath().ToString());

			desc.SetFilterAll(m_Description.Filter);
				
			m_RenderData.Texture = RHITexture::Create(desc);

			m_RenderData.Texture->UpdateSubresource(image.get());
;
			onTake(m_RenderData);
		};

		TOptional<AssetData> data = m_Asset->Load(initTexture);

		if (data)
		{
			initTexture(data.value());
			return true;
		}

		return false;
	}

}
