#include "IonPCH.h"

#include "Core/File/Image.h"
#include "Texture.h"

#include "RHI/RHI.h"
#include "RHI/DX11/DX11Texture.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace Ion
{
	RHITexture* RHITexture::Create(const TextureDescription& desc)
	{
		switch (RHI::GetCurrent())
		{
			case ERHI::OpenGL:
				return new OpenGLTexture(desc);
			case ERHI::DX11:
				return new DX11Texture(desc);
			default:
				return nullptr;
		}
	}

	TShared<RHITexture> RHITexture::CreateShared(const TextureDescription& desc)
	{
		return MakeShareable(Create(desc));
	}

	RHITexture::RHITexture(const TextureDescription& desc)
		: m_Description(desc)
	{
	}

	RHITexture::~RHITexture()
	{
	}
}
