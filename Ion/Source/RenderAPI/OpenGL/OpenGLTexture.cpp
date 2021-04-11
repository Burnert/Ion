#include "IonPCH.h"

#include "OpenGLTexture.h"

namespace Ion
{
	OpenGLTexture::~OpenGLTexture()
	{
	}

	void OpenGLTexture::Bind(uint slot) const
	{
		glBindTextureUnit(slot, m_ID);
	}

	void OpenGLTexture::Unbind() const
	{
		// This is useless for the time being
		// @TODO: Implement a texture manager with some indication which slots are bound by which textures
	}

	OpenGLTexture::OpenGLTexture(File* file)
		: Texture(file)
	{
		CreateTexture();
	}

	OpenGLTexture::OpenGLTexture(Image* image)
		: Texture(image)
	{
		CreateTexture();
	}

	void OpenGLTexture::CreateTexture()
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);

		int width = m_TextureImage->GetWidth();
		int height = m_TextureImage->GetHeight();
		const ubyte* pixelData = m_TextureImage->GetPixelData();

		glTextureStorage2D(m_ID, 1, GL_RGBA8, width, height);

		glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureSubImage2D(m_ID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	}
}
