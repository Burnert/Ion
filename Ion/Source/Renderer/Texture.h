#pragma once

namespace Ion
{
	struct STextureDimensions
	{
		int Width;
		int Height;
	};

	class ION_API Texture
	{
	public:
		static TShared<Texture> Create(File* file);
		static TShared<Texture> Create(Image* image);

		virtual ~Texture();

		virtual void Bind(uint slot = 0) const = 0;
		virtual void Unbind() const = 0;

		FORCEINLINE STextureDimensions GetTextureDimensions() const
		{
			ASSERT(m_TextureImage);
			return STextureDimensions {
				m_TextureImage->GetWidth(),
				m_TextureImage->GetHeight(),
			};
		}

		FORCEINLINE const ubyte* GetPixelData() const
		{
			ASSERT(m_TextureImage);
			return m_TextureImage->GetPixelData();
		}

		FORCEINLINE const Image* GetImage() const
		{
			ASSERT(m_TextureImage);
			return m_TextureImage;
		}

	protected:
		Texture(File* file);
		Texture(Image* image);

	protected:
		Image* m_TextureImage;
	};
}
