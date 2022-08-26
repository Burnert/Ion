#pragma once

#include "Core/Error/Error.h"

#define RHI_BUILD_OPENGL (PLATFORM_SUPPORTS_OPENGL && ENABLE_OPENGL_RHI)
#define RHI_BUILD_DX10   (PLATFORM_SUPPORTS_DX10   && ENABLE_D3D10_RHI)
#define RHI_BUILD_DX11   (PLATFORM_SUPPORTS_DX11   && ENABLE_D3D11_RHI)

#if !PLATFORM_SUPPORTS_OPENGL && !PLATFORM_SUPPORTS_DX10 && !PLATFORM_SUPPORTS_DX11
#error None of the available RHIs is supported on this platform.
#elif !RHI_BUILD_OPENGL && !RHI_BUILD_DX10 && !RHI_BUILD_DX11
#error At least one supported RHI must be enabled.
#endif


namespace Ion
{
	REGISTER_LOGGER(RHILogger, "RHI");

	DEFINE_ERROR_TYPE(RHIError);
}
