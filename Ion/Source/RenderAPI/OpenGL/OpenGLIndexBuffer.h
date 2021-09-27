#pragma once

#include "Renderer/IndexBuffer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLIndexBuffer : public IndexBuffer
	{
		friend class OpenGLRenderer;
	public:
		OpenGLIndexBuffer(uint32* indices, uint32 count);
		virtual ~OpenGLIndexBuffer() override;

		virtual uint32 GetIndexCount() const override;
		virtual uint32 GetTriangleCount() const override;

	protected:
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_Count;
		uint32 m_TriangleCount;
	};
}
