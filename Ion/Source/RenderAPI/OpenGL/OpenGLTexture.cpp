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
		m_BoundSlot = slot;
	}

	void OpenGLTexture::Unbind() const
	{
		// This is useless for the time being
		// @TODO: Implement a texture manager with some indication which slots are bound by which textures
	}

	OpenGLTexture::OpenGLTexture(FileOld* file)
		: Texture(file),
		m_BoundSlot(-1)
	{
		//CreateTexture();
	}

	OpenGLTexture::OpenGLTexture(Image* image)
		: Texture(image),
		m_BoundSlot(-1)
	{
		CreateTexture(image->GetPixelData(), image->GetWidth(), image->GetHeight());
	}

	OpenGLTexture::OpenGLTexture(AssetHandle asset) :
		Texture(asset)
	{
		const AssetTypes::TextureDesc* desc = asset->GetDescription<EAssetType::Texture>();
		CreateTexture(asset->Data(), desc->Width, desc->Height);
	}

	void OpenGLTexture::CreateTexture(const void* const pixelData, uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		ionassert(m_TextureAsset.IsLoaded(), "Texture has not been loaded yet.");

		TRACE_BEGIN(0, "OpenGLTexture - glCreateTextures");
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		TRACE_END(0);

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
