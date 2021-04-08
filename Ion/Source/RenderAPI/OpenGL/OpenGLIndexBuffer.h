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

		// IDrawable:

		virtual void PrepareForDraw() const override;
		virtual uint GetIndexCount() const override;

		// End IDrawable

	private:
		uint m_ID;
		uint m_Count;
	};
}
