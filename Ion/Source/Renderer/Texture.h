#pragma once

namespace Ion
{
	struct TextureDimensions
	{
		uint32 Width;
		uint32 Height;
	};

	class ION_API Texture
	{
	public:
		static TShared<Texture> Create(FileOld* file);
		static TShared<Texture> Create(Image* image);
		static TShared<Texture> Create(AssetHandle asset);

		virtual ~Texture();

		virtual void Bind(uint32 slot = 0) const = 0;
		virtual void Unbind() const = 0;

		FORCEINLINE TextureDimensions GetTextureDimensions() const
		{
			ionassert(m_TextureAsset.IsValid(), "Texture asset is invalid.");

			const AssetTypes::TextureDesc* desc = m_TextureAsset->GetDescription<EAssetType::Texture>();

			return TextureDimensions {
				desc->Width,
				desc->Height,
			};
		}

		FORCEINLINE const uint8* GetPixelData() const
		{
			ionassert(m_TextureAsset.IsValid(), "Texture asset is invalid.");

			return (uint8*)m_TextureAsset->Data();
		}

		FORCEINLINE const Image* GetImage() const
		{
			return nullptr;
		}

	protected:
		Texture(FileOld* file);
		Texture(Image* image);
		Texture(AssetHandle asset);

	protected:
		AssetHandle m_TextureAsset;
	};
}
