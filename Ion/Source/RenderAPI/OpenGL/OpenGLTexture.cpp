#include "IonPCH.h"

#include "OpenGLTexture.h"

namespace Ion
{
	OpenGLTexture::~OpenGLTexture()
	{
		TRACE_FUNCTION();

		glDeleteTextures(1, &m_ID);
		m_ID = 0;
	}

	void OpenGLTexture::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		glBindTextureUnit(slot, m_ID);
	}

	void OpenGLTexture::Unbind() const
	{
		// This is useless for the time being
		// @TODO: Implement a texture manager with some indication which slots are bound by which textures
	}

	OpenGLTexture::OpenGLTexture(FileOld* file)
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
		TRACE_FUNCTION();

		ionassert(m_TextureImage->IsLoaded(), "Image has not been initialized yet.");

		TRACE_BEGIN(0, "OpenGLTexture - glCreateTextures");
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		TRACE_END(0);

		int32 width = m_TextureImage->GetWidth();
		int32 height = m_TextureImage->GetHeight();
		const uint8* pixelData = m_TextureImage->GetPixelData();

		TRACE_BEGIN(1, "OpenGLTexture - glTextureStorage2D");
		glTextureStorage2D(m_ID, 1, GL_RGBA8, width, height);
		TRACE_END(1);

		glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		TRACE_BEGIN(2, "OpenGLTexture - glTextureSubImage2D");
		glTextureSubImage2D(m_ID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		TRACE_END(2);

		TRACE_BEGIN(3, "OpenGLTexture - glGenerateTextureMipmap");
		glGenerateTextureMipmap(m_ID);
		TRACE_END(3);
	}
}
