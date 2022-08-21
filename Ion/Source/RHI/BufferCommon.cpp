#include "IonPCH.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLBuffer.h"
#include "RHI/DX10/DX10Buffer.h"
#include "RHI/DX11/DX11Buffer.h"

namespace Ion
{
	// Vertex Buffer
	RHIVertexBuffer* RHIVertexBuffer::Create(float* vertexAttributes, uint64 count)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return new OpenGLVertexBuffer(vertexAttributes, count);
		case ERHI::DX10:
			return new DX10VertexBuffer(vertexAttributes, count);
		case ERHI::DX11:
			return new DX11VertexBuffer(vertexAttributes, count);
		default:
			return nullptr;
		}
	}

	std::shared_ptr<RHIVertexBuffer> RHIVertexBuffer::CreateShared(float* vertexAttributes, uint64 count)
	{
		return MakeShareable(Create(vertexAttributes, count));
	}

	// Index Buffer
	RHIIndexBuffer* RHIIndexBuffer::Create(uint32* indices, uint32 count)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return new OpenGLIndexBuffer(indices, count);
		case ERHI::DX10:
			return new DX10IndexBuffer(indices, count);
		case ERHI::DX11:
			return new DX11IndexBuffer(indices, count);
		default:
			return nullptr;
		}
	}

	std::shared_ptr<RHIIndexBuffer> RHIIndexBuffer::CreateShared(uint32* indices, uint32 count)
	{
		return MakeShareable(Create(indices, count));
	}

	// Uniform Buffer
	RHIUniformBuffer* RHIUniformBuffer::Create(void* initialData, size_t size)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::DX10:
			return new DX10UniformBuffer(initialData, size);
		case ERHI::DX11:
			return new DX11UniformBuffer(initialData, size);
		case ERHI::OpenGL:
			return new OpenGLUniformBuffer(initialData, size);
		default:
			return nullptr;
		}
	}

	RHIUniformBufferDynamic* RHIUniformBufferDynamic::Create(void* data, size_t size, const UniformDataMap& uniforms)
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::DX10:
			return new DX10UniformBufferDynamic(data, size, uniforms);
		case ERHI::DX11:
			return new DX11UniformBufferDynamic(data, size, uniforms);
		default:
			return nullptr;
		}
	}
}
