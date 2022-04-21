#include "IonPCH.h"

#include "Core/File/Image.h"
#include "Texture.h"

#include "RHI/RHI.h"
#include "RHI/DX11/DX11Texture.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace Ion
{
	TShared<RHITexture> RHITexture::Create(const TextureDescription& desc)
	{
		switch (RHI::GetCurrent())
		{
			case ERHI::OpenGL:
				return MakeShareable(new OpenGLTexture(desc));
			case ERHI::DX11:
				return MakeShareable(new DX11Texture(desc));
			default:
				return TShared<RHITexture>();
		}
	}

	RHITexture::RHITexture(const TextureDescription& desc)
		: m_Description(desc)
	{
	}

	RHITexture::~RHITexture()
	{
	}
}
