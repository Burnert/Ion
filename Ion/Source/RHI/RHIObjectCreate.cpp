#include "IonPCH.h"

#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "Shader.h"

#include "RHI/RHI.h"

#if RHI_BUILD_OPENGL
#include "RHI/OpenGL/OpenGLTexture.h"
#include "RHI/OpenGL/OpenGLBuffer.h"
#include "RHI/OpenGL/OpenGLShader.h"
#endif
#if RHI_BUILD_DX10
#include "RHI/DX10/DX10Texture.h"
#include "RHI/DX10/DX10Buffer.h"
#include "RHI/DX10/DX10Shader.h"
#endif
#if RHI_BUILD_DX11
#include "RHI/DX11/DX11Texture.h"
#include "RHI/DX11/DX11Buffer.h"
#include "RHI/DX11/DX11Shader.h"
#endif

namespace Ion
{
	TRef<RHITexture> RHITexture::Create(const TextureDescription& desc)
	{
		switch (RHI::GetCurrent())
		{
#if RHI_BUILD_OPENGL
		case ERHI::OpenGL:
			return MakeRef<OpenGLTexture>(desc);
#endif
#if RHI_BUILD_DX10
		case ERHI::DX10:
			return MakeRef<DX10Texture>(desc);
#endif
#if RHI_BUILD_DX11
		case ERHI::DX11:
			return MakeRef<DX11Texture>(desc);
#endif
		default:
			return nullptr;
		}
	}

	// Vertex Buffer
	TRef<RHIVertexBuffer> RHIVertexBuffer::Create(float* vertexAttributes, uint64 count)
	{
		switch (RHI::GetCurrent())
		{
#if RHI_BUILD_OPENGL
		case ERHI::OpenGL:
			return MakeRef<OpenGLVertexBuffer>(vertexAttributes, count);
#endif
#if RHI_BUILD_DX10
		case ERHI::DX10:
			return MakeRef<DX10VertexBuffer>(vertexAttributes, count);
#endif
#if RHI_BUILD_DX11
		case ERHI::DX11:
			return MakeRef<DX11VertexBuffer>(vertexAttributes, count);
#endif
		default:
			return nullptr;
		}
	}

	// Index Buffer
	TRef<RHIIndexBuffer> RHIIndexBuffer::Create(uint32* indices, uint32 count)
	{
		switch (RHI::GetCurrent())
		{
#if RHI_BUILD_OPENGL
		case ERHI::OpenGL:
			return MakeRef<OpenGLIndexBuffer>(indices, count);
#endif
#if RHI_BUILD_DX10
		case ERHI::DX10:
			return MakeRef<DX10IndexBuffer>(indices, count);
#endif
#if RHI_BUILD_DX11
		case ERHI::DX11:
			return MakeRef<DX11IndexBuffer>(indices, count);
#endif
		default:
			return nullptr;
		}
	}

	// Uniform Buffer
	TRef<RHIUniformBuffer> RHIUniformBuffer::Create(void* initialData, size_t size)
	{
		switch (RHI::GetCurrent())
		{
#if RHI_BUILD_OPENGL
		case ERHI::OpenGL:
			return MakeRef<OpenGLUniformBuffer>(initialData, size);
#endif
#if RHI_BUILD_DX10
		case ERHI::DX10:
			return MakeRef<DX10UniformBuffer>(initialData, size);
#endif
#if RHI_BUILD_DX11
		case ERHI::DX11:
			return MakeRef<DX11UniformBuffer>(initialData, size);
#endif
		default:
			return nullptr;
		}
	}

	TRef<RHIUniformBufferDynamic> RHIUniformBufferDynamic::Create(void* data, size_t size, const UniformDataMap& uniforms)
	{
		switch (RHI::GetCurrent())
		{
#if RHI_BUILD_DX10
		case ERHI::DX10:
			return MakeRef<DX10UniformBufferDynamic>(data, size, uniforms);
#endif
#if RHI_BUILD_DX11
		case ERHI::DX11:
			return MakeRef<DX11UniformBufferDynamic>(data, size, uniforms);
#endif
		default:
			return nullptr;
		}
	}

	TRef<RHIShader> RHIShader::Create()
	{
		switch (RHI::GetCurrent())
		{
#if RHI_BUILD_OPENGL
		case ERHI::OpenGL:
			return MakeRef<OpenGLShader>();
#endif
#if RHI_BUILD_DX10
		case ERHI::DX10:
			return MakeRef<DX10Shader>();
#endif
#if RHI_BUILD_DX11
		case ERHI::DX11:
			return MakeRef<DX11Shader>();
#endif
		default:
			return nullptr;
		}
	}
}
