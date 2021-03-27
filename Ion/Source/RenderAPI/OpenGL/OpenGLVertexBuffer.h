#pragma once

#include "Renderer/VertexBuffer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(float* vertices, ulong count);
		virtual ~OpenGLVertexBuffer() override;

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		uint m_ID;
	};
}
