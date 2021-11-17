#pragma once

#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"

namespace Ion
{
	class ION_API DX11VertexBuffer : public VertexBuffer
	{
	public:
		DX11VertexBuffer(float* vertexAttributes, uint64 count);
		virtual ~DX11VertexBuffer() override;

		virtual void SetLayout(const TShared<VertexLayout>& layout) override;

		virtual uint32 GetVertexCount() const override;

	protected:
		virtual void Bind() const override;
		virtual void Unbind() const override;

		void BindLayout() const;

	private:
		uint32 m_ID;
		uint32 m_VertexCount;
		TShared<VertexLayout> m_VertexLayout;

		friend class DX11Renderer;
	};

	class ION_API DX11IndexBuffer : public IndexBuffer
	{

	};
}
