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
		const ubyte* Load(File* file);
		FORCEINLINE bool IsLoaded() const { return m_Width && m_Height && m_Channels && m_PixelData; }

		FORCEINLINE const ubyte* GetPixelData() const { return m_PixelData; }
		FORCEINLINE int GetWidth() const { return m_Width; }
		FORCEINLINE int GetHeight() const { return m_Height; }
		FORCEINLINE int GetChannelNum() const { return m_Channels; }

	private:
		ubyte* m_PixelData;
		int m_Width;
		int m_Height;
		int m_Channels;
	};
}
