#include "IonPCH.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLBuffer.h"
#include "RHI/DX11/DX11Buffer.h"

namespace Ion
{
	// Vertex Buffer
	TShared<RHIVertexBuffer> RHIVertexBuffer::Create(float* vertexAttributes, uint64 count)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return MakeShared<OpenGLVertexBuffer>(vertexAttributes, count);
		case ERHI::DX11:
			return MakeShared<DX11VertexBuffer>(vertexAttributes, count);
		default:
			return TShared<RHIVertexBuffer>(nullptr);
		}
	}

	// Index Buffer
	TShared<RHIIndexBuffer> RHIIndexBuffer::Create(uint32* indices, uint32 count)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return MakeShared<OpenGLIndexBuffer>(indices, count);
		case ERHI::DX11:
			return MakeShared<DX11IndexBuffer>(indices, count);
		default:
			return TShared<RHIIndexBuffer>(nullptr);
		}
	}

	// Uniform Buffer
	RHIUniformBuffer* RHIUniformBuffer::Create(void* initialData, size_t size)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::DX11:
			return new DX11UniformBuffer(initialData, size);
		case ERHI::OpenGL:
			return new OpenGLUniformBuffer(initialData, size);
		default:
			return nullptr;
		}
	}

	RHIUniformBuffer* RHIUniformBuffer::Create(void* data, size_t size, const UniformDataMap& uniforms)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::DX11:
			return new DX11UniformBuffer(data, size, uniforms);
		default:
			return nullptr;
		}
	}
}
