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

	struct TextureResourceRenderDataShared
	{
		TShared<RHITexture> Texture;
	};

	struct TextureResourceRenderData
	{
		TWeak<RHITexture> Texture;

		bool IsAvailable() const
		{
			return !Texture.expired();
		}

		TextureResourceRenderDataShared Lock() const
		{
			TextureResourceRenderDataShared data { };
			if (IsAvailable())
			{
				data.Texture = Texture.lock();
			}
			return data;
		}

		TextureResourceRenderData& operator=(const TextureResourceRenderDataShared& shared)
		{
			Texture = shared.Texture;
			return *this;
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
		 * @tparam Lambda params - (const TextureResourceRenderDataShared&)
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
		 * @param asset Asset handle
		 * @param outGuid GUID object to write the resource Guid to.
		 * @param outDescription TextureResourceDescription object to write to
		 * @return True if the file has been parsed successfully.
		 */
		static bool ParseAssetFile(const Asset& asset, GUID& outGuid, TextureResourceDescription& outDescription);

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
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<TextureResourceRenderDataShared>>);

		ionassert(m_Asset);

		if (m_RenderData.IsAvailable())
		{
			onTake(m_RenderData.Lock());
			return true;
		}

		TShared<Image> image = MakeShared<Image>();
		m_Asset->Import(
			[image](TShared<AssetFileMemoryBlock> block)
			{
				AssetImporter::ImportImageAsset(block, *image);
			},
			// Store the pointer (self) so the resource doesn't get deleted before it's loaded
			[this, self = GetPointer(), image, onTake]
			{
				ionassert(m_Asset);
				ionassert(m_Asset->GetType() == EAssetType::Image);

				TextureResourceRenderDataShared sharedRenderData { };

				TextureDescription desc { };
				desc.Dimensions.Width = image->GetWidth();
				desc.Dimensions.Height = image->GetHeight();
				desc.bGenerateMips = true;
				desc.bCreateSampler = true;
				desc.bUseAsRenderTarget = true;
				desc.DebugName = m_Asset->GetDefinitionPath().ToString();

				desc.SetFilterAll(m_Description.Properties.Filter);

				// The shared RHITexture has to reference the resource without actually using a ResourcePtr.
				// This is to make sure the resource won't be deleted before the object is destroyed.
				ResourceMemory::IncRef(*this);

				sharedRenderData.Texture = TShared<RHITexture>(RHITexture::Create(desc), [this](RHITexture* ptr)
				{
					// Decrement the ref count when the actual RHITexture object gets destroyed.
					ResourceMemory::DecRef(*this);
					delete ptr;
				});

				sharedRenderData.Texture->UpdateSubresource(image.get());
				;
				onTake(sharedRenderData);

				m_RenderData = sharedRenderData;
			}
		);
		return (bool)image;
	}

}
