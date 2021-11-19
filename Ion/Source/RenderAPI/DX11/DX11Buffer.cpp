#include "IonPCH.h"

#include "DX11Buffer.h"
#include "DX11Shader.h"

namespace Ion
{
	// -------------------------------------------------------------------
	// DX11VertexBuffer --------------------------------------------------
	// -------------------------------------------------------------------

	DX11VertexBuffer::DX11VertexBuffer(float* vertexAttributes, uint64 count) :
		m_VertexCount(0),
		m_ID(0),
		m_Buffer(nullptr),
		m_InputLayout(nullptr)
	{
		TRACE_FUNCTION();

		ionassert(count <= std::numeric_limits<uint64>::max() / 4);
		ionassert(count * sizeof(float) <= std::numeric_limits<UINT>::max());

		HRESULT hResult = S_OK;

		ID3D11Device* device = DX11::GetDevice();
		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 size = (uint32)(count * sizeof(float));

		D3D11_BUFFER_DESC bd { };
		bd.ByteWidth = size;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd { };
		sd.pSysMem = vertexAttributes;

		dxcall(device->CreateBuffer(&bd, &sd, &m_Buffer),
			"Could not create Vertex Buffer.");
		
		//if (m_Buffer)
		//{
		//	D3D11_MAPPED_SUBRESOURCE ms { };
		//	dxcall(context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms),
		//		"Cannot map the Vertex Buffer.");
		//	memcpy(ms.pData, vertexAttributes, size);
		//	dxcall_v(context->Unmap(m_Buffer, 0));
		//}
	}

	DX11VertexBuffer::~DX11VertexBuffer()
	{
		TRACE_FUNCTION();

		if (m_Buffer)
			m_Buffer->Release();

		if (m_InputLayout)
			m_InputLayout->Release();
	}

	void DX11VertexBuffer::SetLayout(const TShared<VertexLayout>& layout)
	{
		TRACE_FUNCTION();

		ionassert(!m_InputLayout, "The layout has already been set.");

		m_VertexLayout = layout;

		m_IEDArray.reserve(m_VertexLayout->GetAttributes().size());

		for (const VertexAttribute& attribute : m_VertexLayout->GetAttributes())
		{
			D3D11_INPUT_ELEMENT_DESC ied { };
			ied.SemanticName = GetSemanticName(attribute.Semantic);
			ied.SemanticIndex = 0;
			ied.Format = VertexAttributeToDXGIFormat({ attribute.Type, attribute.ElementCount });
			ied.InputSlot = 0;
			ied.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			ied.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			ied.InstanceDataStepRate = 0;

			m_IEDArray.emplace_back(ied);
		}
	}

	void DX11VertexBuffer::CreateDX11Layout(const TShared<DX11Shader>& shader)
	{
		TRACE_FUNCTION();

		ionassert(m_VertexLayout);
		ionassert(!m_IEDArray.empty());

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();

		ID3DBlob* blob = shader->GetVertexShaderByteCode();

		dxcall(device->CreateInputLayout(m_IEDArray.data(), (uint32)m_IEDArray.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &m_InputLayout),
			"Cannot create Input Layout.");
	}

	uint32 DX11VertexBuffer::GetVertexCount() const
	{
		return 0;
	}

	void DX11VertexBuffer::Bind() const
	{
		TRACE_FUNCTION();

		ionassert(m_VertexLayout, "Vertex Layout has not been set.");

		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 stride = m_VertexLayout->GetStride();
		uint32 offset = 0;
		dxcall_v(context->IASetVertexBuffers(0, 1, &m_Buffer, &stride, &offset));
	}

	void DX11VertexBuffer::Unbind() const
	{
		TRACE_FUNCTION();

		dxcall_v(DX11::GetContext()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr));
	}

	void DX11VertexBuffer::BindLayout() const
	{
		TRACE_FUNCTION();

		dxcall_v(DX11::GetContext()->IASetInputLayout(m_InputLayout));
	}

	// -------------------------------------------------------------------
	// DX11IndexBuffer ---------------------------------------------------
	// -------------------------------------------------------------------

	DX11IndexBuffer::DX11IndexBuffer(uint32* indices, uint32 count) :
		m_Count(count),
		m_TriangleCount(count / 3),
		m_ID(0)
	{
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		ID3D11Device* device = DX11::GetDevice();

		uint32 size = (uint32)(count * sizeof(float));

		D3D11_BUFFER_DESC bd { };
		bd.ByteWidth = size;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd { };
		sd.pSysMem = indices;

		dxcall(device->CreateBuffer(&bd, &sd, &m_Buffer),
			"Could not create Index Buffer.");
	}

	DX11IndexBuffer::~DX11IndexBuffer()
	{
		TRACE_FUNCTION();

		if (m_Buffer)
			m_Buffer->Release();
	}

	uint32 DX11IndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	uint32 DX11IndexBuffer::GetTriangleCount() const
	{
		return m_TriangleCount;
	}

	void DX11IndexBuffer::Bind() const
	{
		dxcall_v(DX11::GetContext()->IASetIndexBuffer(m_Buffer, DXGI_FORMAT_R32_UINT, 0));
	}

	void DX11IndexBuffer::Unbind() const
	{
		dxcall_v(DX11::GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
	}
}
