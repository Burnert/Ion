#include "IonPCH.h"

#include "DX11Buffer.h"
#include "DX11.h"

namespace Ion
{
	DX11VertexBuffer::DX11VertexBuffer(float* vertexAttributes, uint64 count) :
		m_VertexCount(0),
		m_ID(0)
	{
		//DX11::GetContext()->IASetVertexBuffers()
	}

	DX11VertexBuffer::~DX11VertexBuffer()
	{

	}

	void DX11VertexBuffer::SetLayout(const TShared<VertexLayout>& layout)
	{
		m_VertexLayout = layout;
	}

	uint32 DX11VertexBuffer::GetVertexCount() const
	{
		return uint32();
	}

	void DX11VertexBuffer::Bind() const
	{

	}

	void DX11VertexBuffer::Unbind() const
	{

	}

	void DX11VertexBuffer::BindLayout() const
	{

	}
}
