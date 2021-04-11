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
		m_TextureImage = new Image();
		bool bLoaded = (bool)m_TextureImage->Load(file);
		ASSERT(bLoaded);
	}

	Texture::Texture(Image* image)
	{
		m_TextureImage = image;
	}
}
