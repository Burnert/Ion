#pragma once

#include "File.h"

namespace Ion
{
	class ION_API Image
	{
	public:
		Image();
		~Image();

		Image(const Image& other);
		Image(Image&& other) noexcept;

		/* Loads an image from the specified file. */
		const uint8* Load(FileOld* file);
		/* Loads an image from the specified file. */
		const uint8* Load(File& file);
		FORCEINLINE bool IsLoaded() const { return m_Width && m_Height && m_Channels && m_PixelData; }

		FORCEINLINE const uint8* GetPixelData() const { return m_PixelData; }
		FORCEINLINE int32 GetWidth() const { return m_Width; }
		FORCEINLINE int32 GetHeight() const { return m_Height; }
		FORCEINLINE int32 GetChannelNum() const { return m_Channels; }

		FORCEINLINE size_t GetPixelDataSize() const { return m_Width * m_Height * m_Channels; }

	private:
		uint8* m_PixelData;
		int32 m_Width;
		int32 m_Height;
		int32 m_Channels;
	};
}
