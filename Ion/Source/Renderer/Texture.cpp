#include "IonPCH.h"

#include "Core/File/Image.h"
#include "Texture.h"

#include "RHI/RHI.h"
#include "RHI/DX11/DX11Texture.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace Ion
{
	TShared<Texture> Texture::Create(const TextureDescription& desc)
	{
		switch (RenderAPI::GetCurrent())
		{
			case ERenderAPI::OpenGL:
				return MakeShareable(new OpenGLTexture(desc));
			case ERenderAPI::DX11:
				return MakeShareable(new DX11Texture(desc));
			default:
				return TShared<Texture>();
		}
	}

	Texture::Texture(const TextureDescription& desc)
		: m_Description(desc)
	{
	}

	Texture::~Texture()
	{
	}
}
