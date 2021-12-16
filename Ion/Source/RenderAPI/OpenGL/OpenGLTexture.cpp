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
		const AssetDescription::Texture* desc = asset->GetDescription<EAssetType::Texture>();
		CreateTexture(asset->Data(), desc->Width, desc->Height);
	}

	void OpenGLTexture::CreateTexture(const void* const pixelData, uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		ionassert(m_TextureAsset->IsLoaded(), "Texture has not been loaded yet.");

		{
			TRACE_SCOPE("OpenGLTexture - glGenTextures | glBindTexture");
			glGenTextures(1, &m_ID);
			glBindTexture(GL_TEXTURE_2D, m_ID);
		}

		{
			TRACE_SCOPE("OpenGLTexture - glTexImage2D");
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 1.0f); // For visualization

		{
			TRACE_SCOPE("OpenGLTexture - glGenerateMipmap");
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
}
