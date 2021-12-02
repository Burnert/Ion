#include "IonPCH.h"

#include "Image.h"
#include "RenderAPI/RenderAPI.h"

#define STBI_NO_STDIO
#include "stb_image.h"

// Return false on fail macros

#define _FAIL(expr) if (!(expr)) { return 0; }
#define _FAIL_M(expr, ...) if (!(expr)) { LOG_ERROR(__VA_ARGS__); return 0; }

namespace Ion
{
	Image::Image() :
		m_PixelData(nullptr),
		m_Width(0),
		m_Height(0),
		m_Channels(0)
	{ }

	Image::~Image()
	{
		if (m_PixelData)
			stbi_image_free(m_PixelData);
	}

	Image::Image(const Image& other)
	{
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;

		// Copy the pixel data
		uint64 pixelDataSize = (uint64)m_Width * (uint64)m_Height * (uint64)m_Channels;
		m_PixelData = (uint8*)malloc(pixelDataSize);
		memcpy_s(m_PixelData, pixelDataSize, other.m_PixelData, pixelDataSize);
	}

	Image::Image(Image&& other) noexcept
	{
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_Channels = other.m_Channels;
		other.m_Width = 0;
		other.m_Height = 0;
		other.m_Channels = 0;

		// Move the pixel data
		m_PixelData = other.m_PixelData;
		other.m_PixelData = nullptr;
	}

	const uint8* Image::Load(FileOld* file)
	{
		TRACE_FUNCTION();

		_FAIL_M(file->Exists(), L"Cannot load image from '{0}'.\nThe file does not exist.", file->GetFilename());

		bool bResult;
		bResult = file->Open(IO::EFileMode::FM_Read);
		_FAIL(bResult);

		int64 fileSize = file->GetSize();
		_FAIL_M(fileSize, L"Cannot load image from '{0}'.\nThe file is empty.", file->GetFilename());

		uint8* data = new uint8[fileSize];
		bResult = file->Read(data, fileSize);
		_FAIL(bResult);

		// OpenGL expects the image to be written in memory from bottom to top
		if (RenderAPI::GetCurrent() == ERenderAPI::OpenGL)
		{
			stbi_set_flip_vertically_on_load(1);
		}

		// Load pixel data with no desired channel number
		m_PixelData = stbi_load_from_memory(data, (int32)fileSize, &m_Width, &m_Height, &m_Channels, 4);
		_FAIL_M(m_PixelData, L"Cannot load pixel data from '{0}'.", file->GetFilename());

		delete[] data;
		file->Close();

		return m_PixelData;
	}

	const uint8* Image::Load(File& file)
	{
		TRACE_FUNCTION();

		_FAIL_M(file.Exists(), L"Cannot load image from '{0}'.\nThe file does not exist.", file.GetFullPath());

		bool bResult;
		bResult = file.Open(EFileMode::Read);
		_FAIL(bResult);

		int64 fileSize = file.GetSize();
		_FAIL_M(fileSize, L"Cannot load image from '{0}'.\nThe file is empty.", file.GetFullPath());

		uint8* data = new uint8[fileSize];
		bResult = file.Read(data, fileSize);
		_FAIL(bResult);

		stbi_set_flip_vertically_on_load(1);

		// Load pixel data with no desired channel number
		m_PixelData = stbi_load_from_memory(data, (int32)fileSize, &m_Width, &m_Height, &m_Channels, 4);
		_FAIL_M(m_PixelData, L"Cannot load pixel data from '{0}'.", file.GetFullPath());
		m_Channels = 4;

		delete[] data;
		file.Close();

		return m_PixelData;
	}
}
