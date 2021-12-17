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

		ionassertnd(vertexAttributes);
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

		COMRelease(m_Buffer);
		COMRelease(m_InputLayout);
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

	void DX11VertexBuffer::SetLayoutShader(const TShared<Shader>& shader)
	{
		CreateDX11Layout(TStaticCast<DX11Shader>(shader));
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
			"Could not create Input Layout.");
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

		ionassertnd(indices);

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

		COMRelease(m_Buffer);
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

	// -------------------------------------------------------------------
	// DX11UniformBuffer -------------------------------------------------
	// -------------------------------------------------------------------

#pragma warning(disable:6387)

	DX11UniformBuffer::DX11UniformBuffer(void* initialData, size_t size) :
		m_Data(nullptr),
		m_DataSize(size),
		m_Buffer(nullptr)
	{
		TRACE_FUNCTION();

		ionassertnd(initialData);
		ionassert(size <= std::numeric_limits<UINT>::max());

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();

		// Align to 16 byte boundaries for a faster memcpy to a mapped buffer.
		m_Data = _aligned_malloc(size, 16);
		memcpy(m_Data, initialData, size);

		D3D11_BUFFER_DESC bd { };
		bd.ByteWidth = (uint32)size;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd { };
		sd.pSysMem = m_Data;

		dxcall(device->CreateBuffer(&bd, &sd, &m_Buffer),
			"Could not create Constant Buffer.");
	}

	DX11UniformBuffer::DX11UniformBuffer(void* data, size_t size, const UniformDataMap& uniforms) :
		UniformBuffer(uniforms),
		m_Data(nullptr),
		m_DataSize(size),
		m_Buffer(nullptr)
	{
		// @TODO: yes
	}

	DX11UniformBuffer::~DX11UniformBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);

		_aligned_free(m_Data);
	}

	void DX11UniformBuffer::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		ionassert(m_Buffer);

		ID3D11DeviceContext* context = DX11::GetContext();

		context->VSSetConstantBuffers(slot, 1, &m_Buffer);
		context->PSSetConstantBuffers(slot, 1, &m_Buffer);
	}

	void* DX11UniformBuffer::GetDataPtr() const
	{
		return m_Data;
	}

	void DX11UniformBuffer::UpdateData() const
	{
		TRACE_FUNCTION();

		ionassert(m_Buffer);

		HRESULT hResult;

		ID3D11DeviceContext* context = DX11::GetContext();

		D3D11_MAPPED_SUBRESOURCE msd { };
		dxcall(context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msd));
		memcpy(msd.pData, m_Data, m_DataSize);
		dxcall_v(context->Unmap(m_Buffer, 0));
	}
}
