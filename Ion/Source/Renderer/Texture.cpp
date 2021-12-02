#include "IonPCH.h"

#include "Core/File/Image.h"
#include "Texture.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLTexture.h"
#include "RenderAPI/DX11/DX11Texture.h"

namespace Ion
{
	TShared<Texture> Texture::Create(FileOld* file)
	{
		return MakeShareable(new OpenGLTexture(file));
	}

	TShared<Texture> Texture::Create(Image* image)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShareable(new OpenGLTexture(image));
		case ERenderAPI::DX11:
			return MakeShareable(new DX11Texture(image));
		default:
			return TShared<Texture>(nullptr);
		}
	}

	TShared<Texture> Texture::Create(AssetHandle asset)
	{
		switch (RenderAPI::GetCurrent())
		{
			case ERenderAPI::OpenGL:
				return MakeShareable(new OpenGLTexture(asset));
			case ERenderAPI::DX11:
				return MakeShareable(new DX11Texture(asset));
			default:
				return TShared<Texture>(nullptr);
		}
	}

	Texture::~Texture()
	{
		//if (m_TextureImage)
		//{
		//	delete m_TextureImage;
		//}
	}

	Texture::Texture(FileOld* file)
	{
		TRACE_FUNCTION();

		//m_TextureImage = new Image();
		//bool bLoaded = (bool)m_TextureImage->Load(file);
		//ionassert(bLoaded, "The specified file could not be loaded!");
	}

	Texture::Texture(Image* image)
	{
		//m_TextureImage = image;
	}

	Texture::Texture(AssetHandle asset) :
		m_TextureAsset(asset)
	{
	}
}
