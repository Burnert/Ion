#include "IonPCH.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLBuffer.h"
#include "RenderAPI/DX11/DX11Buffer.h"

namespace Ion
{
	// Vertex Buffer
	TShared<VertexBuffer> VertexBuffer::Create(float* vertexAttributes, uint64 count)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLVertexBuffer>(vertexAttributes, count);
		case ERenderAPI::DX11:
			return MakeShared<DX11VertexBuffer>(vertexAttributes, count);
		default:
			return TShared<VertexBuffer>(nullptr);
		}
	}

	// Index Buffer
	TShared<IndexBuffer> IndexBuffer::Create(uint32* indices, uint32 count)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLIndexBuffer>(indices, count);
		case ERenderAPI::DX11:
			return MakeShared<DX11IndexBuffer>(indices, count);
		default:
			return TShared<IndexBuffer>(nullptr);
		}
	}

	// Uniform Buffer
	UniformBuffer* UniformBuffer::Create(void* initialData, size_t size)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::DX11:
			return new DX11UniformBuffer(initialData, size);
		case ERenderAPI::OpenGL:
			return new OpenGLUniformBuffer(initialData, size);
		default:
			return nullptr;
		}
	}

	UniformBuffer* UniformBuffer::Create(void* data, size_t size, const UniformDataMap& uniforms)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::DX11:
			return new DX11UniformBuffer(data, size, uniforms);
		default:
			return nullptr;
		}
	}
}
