#include "IonPCH.h"

#include "Texture.h"

#include "RHI/RHI.h"
#include "RHI/DX10/DX10Texture.h"
#include "RHI/DX11/DX11Texture.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace Ion
{
	//RHITexture* RHITexture::Create(const TextureDescription& desc)
	//{
	//	switch (RHI::GetCurrent())
	//	{
	//		case ERHI::OpenGL:
	//			return new OpenGLTexture(desc);
	//		case ERHI::DX10:
	//			return new DX10Texture(desc);
	//		case ERHI::DX11:
	//			return new DX11Texture(desc);
	//		default:
	//			return nullptr;
	//	}
	//}

	//std::shared_ptr<RHITexture> RHITexture::CreateShared(const TextureDescription& desc)
	//{
	//	return std::shared_ptr<RHITexture>(Create(desc));
	//}

	TRef<RHITexture> RHITexture::CreateRef(const TextureDescription& desc)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return MakeRef<OpenGLTexture>(desc);
		case ERHI::DX10:
			return MakeRef<DX10Texture>(desc);
		case ERHI::DX11:
			return MakeRef<DX11Texture>(desc);
		default:
			return nullptr;
		}
	}

	RHITexture::RHITexture(const TextureDescription& desc)
		: m_Description(desc)
	{
		ionassert(!desc.DebugName.empty(), "Specify a debug name to avoid future problems.");
	}

	RHITexture::~RHITexture()
	{
	}
}
