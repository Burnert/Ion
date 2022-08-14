#include "IonPCH.h"

#include "DX10Buffer.h"
#include "DX10Shader.h"

namespace Ion
{
	// -------------------------------------------------------------------
	// DX10VertexBuffer --------------------------------------------------
	// -------------------------------------------------------------------

	DX10VertexBuffer::DX10VertexBuffer(float* vertexAttributes, uint64 count) :
		m_VertexCount(0),
		m_ID(0),
		m_Buffer(nullptr),
		m_InputLayout(nullptr)
	{
		TRACE_FUNCTION();

		ionverify(vertexAttributes);
		ionassert(count <= std::numeric_limits<uint64>::max() / 4);
		ionassert(count * sizeof(float) <= std::numeric_limits<UINT>::max());

		HRESULT hResult = S_OK;

		ID3D10Device* device = DX10::GetDevice();

		uint32 size = (uint32)(count * sizeof(float));

		D3D10_BUFFER_DESC bd { };
		bd.ByteWidth = size;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;

		D3D10_SUBRESOURCE_DATA sd { };
		sd.pSysMem = vertexAttributes;

		dxcall(device->CreateBuffer(&bd, &sd, &m_Buffer),
			"Could not create Vertex Buffer.");

		// @TODO: SetDebugName on buffers
		DX10Logger.Trace("Created DX10VertexBuffer object.");
	}

	DX10VertexBuffer::~DX10VertexBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);
		COMRelease(m_InputLayout);
	}

	void DX10VertexBuffer::SetLayout(const TShared<RHIVertexLayout>& layout)
	{
		TRACE_FUNCTION();

		ionassert(!m_InputLayout, "The layout has already been set.");

		m_VertexLayout = layout;

		m_IEDArray.reserve(m_VertexLayout->GetAttributes().size());

		for (const VertexAttribute& attribute : m_VertexLayout->GetAttributes())
		{
			D3D10_INPUT_ELEMENT_DESC ied { };
			ied.SemanticName = DXCommon::GetSemanticName(attribute.Semantic);
			ied.SemanticIndex = 0;
			ied.Format = DXCommon::VertexAttributeToDXGIFormat({ attribute.Type, attribute.ElementCount });
			ied.InputSlot = 0;
			ied.AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
			ied.InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
			ied.InstanceDataStepRate = 0;

			m_IEDArray.emplace_back(ied);
		}
	}

	void DX10VertexBuffer::SetLayoutShader(const TShared<RHIShader>& shader)
	{
		CreateDX10Layout(TStaticCast<DX10Shader>(shader));
	}

	void DX10VertexBuffer::CreateDX10Layout(const TShared<DX10Shader>& shader)
	{
		TRACE_FUNCTION();

		ionassert(shader);
		ionassert(shader->IsCompiled());
		ionassert(m_VertexLayout);
		ionassert(!m_IEDArray.empty());

		HRESULT hResult;

		ID3D10Device* device = DX10::GetDevice();

		ID3DBlob* blob = shader->GetVertexShaderByteCode();

		dxcall(device->CreateInputLayout(m_IEDArray.data(), (uint32)m_IEDArray.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &m_InputLayout),
			"Could not create Input Layout.");

		DX10Logger.Trace("Created DX10VertexBuffer Input Layout object.");
	}

	uint32 DX10VertexBuffer::GetVertexCount() const
	{
		return 0;
	}

	void DX10VertexBuffer::Bind() const
	{
		ionassert(m_VertexLayout, "Vertex Layout has not been set.");

		ID3D10Device* device = DX10::GetDevice();

		uint32 stride = m_VertexLayout->GetStride();
		uint32 offset = 0;
		dxcall_v(device->IASetVertexBuffers(0, 1, &m_Buffer, &stride, &offset));
	}

	void DX10VertexBuffer::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr));
	}

	void DX10VertexBuffer::BindLayout() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->IASetInputLayout(m_InputLayout));
	}

	// -------------------------------------------------------------------
	// DX10IndexBuffer ---------------------------------------------------
	// -------------------------------------------------------------------

	DX10IndexBuffer::DX10IndexBuffer(uint32* indices, uint32 count) :
		m_Count(count),
		m_TriangleCount(count / 3),
		m_ID(0)
	{
		TRACE_FUNCTION();

		ionverify(indices);

		HRESULT hResult = S_OK;

		ID3D10Device* device = DX10::GetDevice();

		uint32 size = (uint32)(count * sizeof(float));

		D3D10_BUFFER_DESC bd { };
		bd.ByteWidth = size;
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;

		D3D10_SUBRESOURCE_DATA sd { };
		sd.pSysMem = indices;

		dxcall(device->CreateBuffer(&bd, &sd, &m_Buffer),
			"Could not create Index Buffer.");

		DX10Logger.Trace("Created DX10IndexBuffer object.");
	}

	DX10IndexBuffer::~DX10IndexBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);
	}

	uint32 DX10IndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	uint32 DX10IndexBuffer::GetTriangleCount() const
	{
		return m_TriangleCount;
	}

	void DX10IndexBuffer::Bind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->IASetIndexBuffer(m_Buffer, DXGI_FORMAT_R32_UINT, 0));
	}

	void DX10IndexBuffer::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
	}

	// -------------------------------------------------------------------
	// DX10UniformBuffer -------------------------------------------------
	// -------------------------------------------------------------------

#pragma warning(disable:6387)

	// Common Uniform Buffer -------------------------------------------------------

	_DX10UniformBufferCommon::_DX10UniformBufferCommon(void* initialData, size_t size) :
		DataSize(size),
		Buffer(nullptr)
	{
		TRACE_FUNCTION();

		ionverify(initialData);
		ionassert(size <= std::numeric_limits<UINT>::max());

		HRESULT hResult;

		ID3D10Device* device = DX10::GetDevice();

		// Align to 16 byte boundaries for a faster memcpy to a mapped buffer.
		Data = _aligned_malloc(size, 16);
		memcpy(Data, initialData, size);

		D3D10_BUFFER_DESC bd { };
		bd.ByteWidth = (uint32)AlignAs(size, 16);
		bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D10_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

		D3D10_SUBRESOURCE_DATA sd { };
		sd.pSysMem = Data;

		dxcall(device->CreateBuffer(&bd, &sd, &Buffer),
			"Could not create Constant Buffer.");
	}

	_DX10UniformBufferCommon::~_DX10UniformBufferCommon()
	{
		TRACE_FUNCTION();

		COMRelease(Buffer);

		_aligned_free(Data);
	}

	void _DX10UniformBufferCommon::Bind(uint32 slot) const
	{
		ionassert(Buffer);

		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->VSSetConstantBuffers(slot, 1, &Buffer));
		dxcall_v(device->PSSetConstantBuffers(slot, 1, &Buffer));
	}

	void _DX10UniformBufferCommon::UpdateData() const
	{
		// Might become useful
		//TRACE_FUNCTION();

		ionassert(Buffer);

		HRESULT hResult;

		void* pData = nullptr;
		dxcall(Buffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &pData));
		memcpy(pData, Data, DataSize);
		dxcall_v(Buffer->Unmap());
	}

	// Specific Uniform Buffer -------------------------------------------------------

	DX10UniformBuffer::DX10UniformBuffer(void* initialData, size_t size) :
		m_Common(initialData, size)
	{
		DX10Logger.Trace("Created DX10UniformBuffer object.");
	}

	void DX10UniformBuffer::Bind(uint32 slot) const
	{
		m_Common.Bind(slot);
	}

	void* DX10UniformBuffer::GetDataPtr() const
	{
		return m_Common.Data;
	}

	void DX10UniformBuffer::UpdateData() const
	{
		m_Common.UpdateData();
	}

	DX10UniformBufferDynamic::DX10UniformBufferDynamic(void* initialData, size_t size, const UniformDataMap& uniforms) :
		RHIUniformBufferDynamic(uniforms),
		m_Common(initialData, size)
	{
		DX10Logger.Trace("Created DX10UniformBufferDynamic object.");
	}

	void DX10UniformBufferDynamic::Bind(uint32 slot) const
	{
		m_Common.Bind(slot);
	}

	const UniformData* DX10UniformBufferDynamic::GetUniformData(const String& name) const
	{
		auto it = GetUniformDataMap().find(name);
		if (it == GetUniformDataMap().end())
			return nullptr;
		return &it->second;
	}

	void DX10UniformBufferDynamic::UpdateData() const
	{
		m_Common.UpdateData();
	}

	bool DX10UniformBufferDynamic::SetUniformValue_Internal(const String& name, const void* value)
	{
		const UniformData* uniform = GetUniformData(name);
		if (!uniform)
			return false;

		void* fieldAddress = (uint8*)m_Common.Data + uniform->Offset;
		size_t fieldSize = GetUniformTypeSize(uniform->Type);

		ionassert(fieldAddress);
		ionassert(fieldSize);

		memcpy(fieldAddress, value, fieldSize);

		return true;
	}

	void* DX10UniformBufferDynamic::GetUniformAddress(const String& name) const
	{
		auto it = GetUniformDataMap().find(name);
		if (it == GetUniformDataMap().end())
			return nullptr;

		return (uint8*)m_Common.Data + it->second.Offset;
	}
}
