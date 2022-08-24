#include "IonPCH.h"

#include "OpenGLTexture.h"
#include "OpenGLRenderer.h"

namespace Ion
{
	OpenGLTexture::~OpenGLTexture()
	{
		TRACE_FUNCTION();

		ReleaseTexture();

		if (m_FramebufferID != (uint32)-1)
		{
			glDeleteFramebuffers(1, &m_FramebufferID);
		}
	}

	Result<void, RHIError> OpenGLTexture::SetDimensions(TextureDimensions dimensions)
	{
		return Ok();
	}

	Result<void, RHIError> OpenGLTexture::UpdateSubresource(Image* image)
	{
		TRACE_FUNCTION();

		if (!image->IsLoaded())
		{
			OpenGLLogger.Error("Cannot Update Subresource of Texture. Image has not been loaded.");
			ionthrow(RHIError);
		}

		// Dimensions are different
		if (image->GetWidth() != m_Description.Dimensions.Width ||
			image->GetHeight() != m_Description.Dimensions.Height)
		{
			OpenGLLogger.Warn("Image dimensions do not match texture dimensions.");
			//ReleaseTexture();
			//CreateTexture(image->GetPixelData(), image->GetWidth(), image->GetHeight());
			ionthrow(RHIError);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->GetWidth(), image->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->GetPixelData());

		if (m_Description.bGenerateMips)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		return Ok();
	}

	Result<void, RHIError> OpenGLTexture::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		glBindTextureUnit(slot, m_ID);
		m_BoundSlot = slot;

		return Ok();
	}

	Result<void, RHIError> OpenGLTexture::Unbind() const
	{
		// This is useless for the time being
		// @TODO: Implement a texture manager with some indication which slots are bound by which textures

		return Ok();
	}

	Result<void, RHIError> OpenGLTexture::CopyTo(const TRef<RHITexture>& destination) const
	{
		return Ok();
	}

	Result<void, RHIError> OpenGLTexture::Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType)
	{
		return Ok();
	}

	Result<void, RHIError> OpenGLTexture::Unmap()
	{
		return Ok();
	}

	void* OpenGLTexture::GetNativeID() const
	{
		return (void*)(uint64)m_ID;
	}

	OpenGLTexture::OpenGLTexture(const TextureDescription& desc) :
		RHITexture(desc),
		m_BoundSlot(-1),
		m_ID((uint32)-1),
		m_FramebufferID((uint32)-1)
	{
		CreateTexture(desc);
	}

	void OpenGLTexture::CreateTexture(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		{
			TRACE_SCOPE("OpenGLTexture - glGenTextures | glBindTexture");
			glGenTextures(1, &m_ID);
			glBindTexture(GL_TEXTURE_2D, m_ID);
		}

		//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA, desc.Dimensions.Width, desc.Dimensions.Height);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, desc.Dimensions.Width, desc.Dimensions.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		//{
		//	TRACE_SCOPE("OpenGLTexture - glTexImage2D");
		//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		//}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SelectGLFilterType(desc.MinFilter, desc.bGenerateMips, desc.MipFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SelectGLFilterType(desc.MagFilter, desc.bGenerateMips, desc.MipFilter));
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, desc.LODBias);

		//{
		//	TRACE_SCOPE("OpenGLTexture - glGenerateMipmap");
		//	glGenerateMipmap(GL_TEXTURE_2D);
		//}

		if (desc.bUseAsRenderTarget)
		{
			TRACE_SCOPE("OpenGLTexture - Generate framebuffers");

			uint32 currentFramebuffer = ((OpenGLRenderer*)Renderer::Get())->GetCurrentRenderTarget();

			glGenFramebuffers(1, &m_FramebufferID);
			glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ID, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				OpenGLLogger.Error("Framebuffer is not complete.");
				debugbreak();
			}

			glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
		}
	}

	void OpenGLTexture::ReleaseTexture()
	{
		TRACE_FUNCTION();

		glDeleteTextures(1, &m_ID);
		m_ID = 0;
	}
}
