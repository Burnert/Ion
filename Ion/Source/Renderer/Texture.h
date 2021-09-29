#pragma once

namespace Ion
{
	struct TextureDimensions
	{
		int32 Width;
		int32 Height;
	};

	class ION_API Texture
	{
	public:
		static TShared<Texture> Create(File* file);
		static TShared<Texture> Create(Image* image);

		virtual ~Texture();

		virtual void Bind(uint32 slot = 0) const = 0;
		virtual void Unbind() const = 0;

		FORCEINLINE TextureDimensions GetTextureDimensions() const
		{
			ionassert(m_TextureImage, "Texture image is not set!");
			return TextureDimensions {
				m_TextureImage->GetWidth(),
				m_TextureImage->GetHeight(),
			};
		}

		FORCEINLINE const uint8* GetPixelData() const
		{
			ionassert(m_TextureImage, "Texture image is not set!");
			return m_TextureImage->GetPixelData();
		}

		FORCEINLINE const Image* GetImage() const
		{
			return m_TextureImage;
		}

	protected:
		Texture(File* file);
		Texture(Image* image);

	protected:
		Image* m_TextureImage;
	};
}
