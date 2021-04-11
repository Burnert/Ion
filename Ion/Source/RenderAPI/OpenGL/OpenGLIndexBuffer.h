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

		virtual uint GetIndexCount() const override;
		virtual uint GetTriangleCount() const override;

	private:
		uint m_ID;
		uint m_Count;
		uint m_TriangleCount;
	};
}
