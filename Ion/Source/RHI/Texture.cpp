#include "IonPCH.h"

#include "Texture.h"

#include "RHI/RHI.h"
#include "RHI/DX10/DX10Texture.h"
#include "RHI/DX11/DX11Texture.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace Ion
{
	TRef<RHITexture> RHITexture::Create(const TextureDescription& desc)
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

		RHILogger.Info("RHITexture \"{}\" object has been created.", desc.DebugName);
	}

	RHITexture::~RHITexture()
	{
		RHILogger.Info("RHITexture \"{}\" object has been destroyed.", m_Description.DebugName);
	}
}
