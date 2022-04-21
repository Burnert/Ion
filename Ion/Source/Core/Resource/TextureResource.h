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
		/**
		 * @brief Query the Resource Manager for a Texture Resource
		 * associated with the specified asset.
		 * 
		 * @see Resource::Query(const Asset& asset)
		 * 
		 * @param asset Asset associated with the Resource
		 * @return Shared pointer to the Resource
		 */
		static TShared<TextureResource> Query(const Asset& asset);

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

	protected:
		TextureResource(const Asset& asset) :
			Resource(asset)
		{
		}

	private:
		/**
		 * @brief Parses the TextureResource node in the .iasset file
		 * 
		 * @param path .iasset file path
		 * @param outDescription TextureResourceDescription object to write to
		 * @return True if the file has been parsed successfully.
		 */
		static bool ParseAssetFile(const FilePath& path, TextureResourceDescription& outDescription);

	private:
		TextureResourceRenderData m_RenderData;

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

			// Read the asset file and set the texture description to match the file.
			TextureResourceDescription resDesc { };
			if (ParseAssetFile(m_Asset->GetDefinitionPath(), resDesc))
			{
				desc.SetFilterAll(resDesc.Filter);
			}

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
