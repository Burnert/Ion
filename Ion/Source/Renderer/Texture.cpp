#include "IonPCH.h"

#include "Core/File/Image.h"
#include "Texture.h"

#include "RenderAPI/OpenGL/OpenGLTexture.h"

namespace Ion
{
	TShared<Texture> Texture::Create(File* file)
	{
		return MakeShareable(new OpenGLTexture(file));
	}

	TShared<Texture> Texture::Create(Image* image)
	{
		return MakeShareable(new OpenGLTexture(image));
	}

	Texture::~Texture()
	{
		if (m_TextureImage)
		{
			delete m_TextureImage;
		}
	}

	Texture::Texture(File* file)
	{
		TRACE_FUNCTION();

		m_TextureImage = new Image();
		bool bLoaded = (bool)m_TextureImage->Load(file);
		ionassert(bLoaded, "The specified file could not be loaded!");
	}

	Texture::Texture(Image* image)
	{
		m_TextureImage = image;
	}
}