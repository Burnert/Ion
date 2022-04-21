#pragma once

#include "Resource.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Texture Resource
	// ------------------------------------------------------------

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
		 * @tparam Lambda Must be convertible to TFuncResourceOnTake<Texture>
		 * @see TFuncResourceOnTake
		 * 
		 * @param onTake If the resource is ready, called immediately,
		 * else, called when the asset and the resource get loaded.
		 * 
		 * @return Returns true if the resource is available instantly.
		 */
		template<typename Lambda>
		bool Take(Lambda onTake);

	protected:
		TextureResource(const Asset& asset) :
			Resource(asset)
		{
		}

	private:
		TShared<RHITexture> m_Texture;

		friend class Resource;
	};

	template<typename Lambda>
	inline bool TextureResource::Take(Lambda onTake)
	{
		if (m_Texture)
		{
			onTake(m_Texture);
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

			m_Texture = RHITexture::Create(desc);

			onTake(m_Texture);
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
