#pragma once

#include "Renderer/IndexBuffer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint* indices, uint count);
		virtual ~OpenGLIndexBuffer() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint m_ID;
	};
}
