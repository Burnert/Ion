#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_DX11

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
		DX11Logger.Info("DX11VertexBuffer has been created.");

		CreateBuffer(vertexAttributes, count)
			.Err([](Error& error) { DX11Logger.Critical("Cannot create a Vertex Buffer.\n{}", error.Message); })
			.Unwrap();
		
		//if (m_Buffer)
		//{
		//	D3D11_MAPPED_SUBRESOURCE ms { };
		//	dxcall(context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms),
		//		"Cannot map the Vertex Buffer.");
		//	memcpy(ms.pData, vertexAttributes, size);
		//	dxcall(context->Unmap(m_Buffer, 0));
		//}
	}

	DX11VertexBuffer::~DX11VertexBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);
		COMRelease(m_InputLayout);

		DX11Logger.Info("DX11VertexBuffer has been destroyed.");
	}

	void DX11VertexBuffer::SetLayout(const TRef<RHIVertexLayout>& layout)
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
			ied.Format = DXCommon::VertexAttributeToDXGIFormat({ attribute.Type, attribute.ElementCount });
			ied.InputSlot = 0;
			ied.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			ied.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			ied.InstanceDataStepRate = 0;

			m_IEDArray.emplace_back(ied);
		}
	}

	Result<void, RHIError> DX11VertexBuffer::SetLayoutShader(const TRef<RHIShader>& shader)
	{
		return CreateDX11Layout(RefCast<DX11Shader>(shader));
	}

	Result<void, RHIError> DX11VertexBuffer::CreateDX11Layout(const TRef<DX11Shader>& shader)
	{
		TRACE_FUNCTION();

		ionassert(shader);
		ionassert(shader->IsCompiled());
		ionassert(m_VertexLayout);
		ionassert(!m_IEDArray.empty());

		ID3D11Device* device = DX11::GetDevice();

		ID3DBlob* blob = shader->GetVertexShaderByteCode();

		dxcall(device->CreateInputLayout(m_IEDArray.data(), (uint32)m_IEDArray.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &m_InputLayout),
			"Could not create Input Layout.");

		DX11Logger.Debug("DX11VertexBuffer Input Layout object has been created.");

		return Ok();
	}

	uint32 DX11VertexBuffer::GetVertexCount() const
	{
		// @TODO: 0...
		return 0;
	}

	Result<void, RHIError> DX11VertexBuffer::Bind() const
	{
		ionassert(m_VertexLayout, "Vertex Layout has not been set.");

		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 stride = m_VertexLayout->GetStride();
		uint32 offset = 0;
		dxcall(context->IASetVertexBuffers(0, 1, &m_Buffer, &stride, &offset));

		return Ok();
	}

	Result<void, RHIError> DX11VertexBuffer::Unbind() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr));

		return Ok();
	}

	Result<void, RHIError> DX11VertexBuffer::BindLayout() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->IASetInputLayout(m_InputLayout));

		return Ok();
	}

	Result<void, RHIError> DX11VertexBuffer::CreateBuffer(float* vertexAttributes, uint64 count)
	{
		TRACE_FUNCTION();

		ionverify(vertexAttributes);
		ionassert(count <= std::numeric_limits<uint64>::max() / 4);
		ionassert(count * sizeof(float) <= std::numeric_limits<UINT>::max());

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

		// @TODO: SetDebugName on buffers
		DX11Logger.Debug("DX11VertexBuffer Buffer object has been created.");

		return Ok();
	}

	// -------------------------------------------------------------------
	// DX11IndexBuffer ---------------------------------------------------
	// -------------------------------------------------------------------

	DX11IndexBuffer::DX11IndexBuffer(uint32* indices, uint32 count) :
		m_Count(count),
		m_TriangleCount(count / 3),
		m_ID(0)
	{
		DX11Logger.Info("DX11IndexBuffer has been created.");

		CreateBuffer(indices, count)
			.Err([](Error& error) { DX11Logger.Critical("Cannot create an Index Buffer.\n{}", error.Message); })
			.Unwrap();
	}

	DX11IndexBuffer::~DX11IndexBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);

		DX11Logger.Info("DX11IndexBuffer has been destroyed.");
	}

	uint32 DX11IndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	uint32 DX11IndexBuffer::GetTriangleCount() const
	{
		return m_TriangleCount;
	}

	Result<void, RHIError> DX11IndexBuffer::Bind() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->IASetIndexBuffer(m_Buffer, DXGI_FORMAT_R32_UINT, 0));

		return Ok();
	}

	Result<void, RHIError> DX11IndexBuffer::Unbind() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));

		return Ok();
	}

	Result<void, RHIError> DX11IndexBuffer::CreateBuffer(uint32* indices, uint64 count)
	{
		TRACE_FUNCTION();

		ionverify(indices);

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

		DX11Logger.Debug("DX11IndexBuffer Buffer object has been created.");
		
		return Ok();
	}

	// -------------------------------------------------------------------
	// DX11UniformBuffer -------------------------------------------------
	// -------------------------------------------------------------------

#pragma warning(disable:6387)

	// Common Uniform Buffer -------------------------------------------------------

	_DX11UniformBufferCommon::_DX11UniformBufferCommon(void* initialData, size_t size) :
		DataSize(size),
		Buffer(nullptr)
	{
		DX11Logger.Debug("DX11UniformBufferCommon has been created.");

		CreateBuffer(initialData, size)
			.Err([](Error& error) { DX11Logger.Critical("Cannot create a Uniform Buffer.\n{}", error.Message); })
			.Unwrap();
	}

	_DX11UniformBufferCommon::~_DX11UniformBufferCommon()
	{
		TRACE_FUNCTION();

		COMRelease(Buffer);

		_aligned_free(Data);

		DX11Logger.Debug("DX11UniformBufferCommon has been destroyed.");
	}

	Result<void, RHIError> _DX11UniformBufferCommon::Bind(uint32 slot) const
	{
		ionassert(Buffer);

		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->VSSetConstantBuffers(slot, 1, &Buffer));
		dxcall(context->PSSetConstantBuffers(slot, 1, &Buffer));

		return Ok();
	}

	Result<void, RHIError> _DX11UniformBufferCommon::UpdateData() const
	{
		// Might become useful
		//TRACE_FUNCTION();

		ionassert(Buffer);

		ID3D11DeviceContext* context = DX11::GetContext();

		D3D11_MAPPED_SUBRESOURCE msd { };
		dxcall(context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msd));
		memcpy(msd.pData, Data, DataSize);
		dxcall(context->Unmap(Buffer, 0));

		return Ok();
	}

	Result<void, RHIError> _DX11UniformBufferCommon::CreateBuffer(void* initialData, size_t size)
	{
		TRACE_FUNCTION();

		ionverify(initialData);
		ionassert(size <= std::numeric_limits<UINT>::max());

		ID3D11Device* device = DX11::GetDevice();

		// Align to 16 byte boundaries for a faster memcpy to a mapped buffer.
		Data = _aligned_malloc(size, 16);
		memcpy(Data, initialData, size);

		D3D11_BUFFER_DESC bd { };
		bd.ByteWidth = (uint32)AlignAs(size, 16);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd { };
		sd.pSysMem = Data;

		dxcall(device->CreateBuffer(&bd, &sd, &Buffer),
			"Could not create Constant Buffer.");

		DX11Logger.Debug("DX11UniformBufferCommon Buffer object has been created.");

		return Ok();
	}


	// Specific Uniform Buffer -------------------------------------------------------

	DX11UniformBuffer::DX11UniformBuffer(void* initialData, size_t size) :
		m_Common(initialData, size)
	{
		DX11Logger.Info("DX11UniformBuffer has been created.");
	}

	DX11UniformBuffer::~DX11UniformBuffer()
	{
		DX11Logger.Info("DX11UniformBuffer has been destroyed.");
	}

	Result<void, RHIError> DX11UniformBuffer::Bind(uint32 slot) const
	{
		return m_Common.Bind(slot);
	}

	void* DX11UniformBuffer::GetDataPtr() const
	{
		return m_Common.Data;
	}

	Result<void, RHIError> DX11UniformBuffer::UpdateData() const
	{
		return m_Common.UpdateData();
	}

	DX11UniformBufferDynamic::DX11UniformBufferDynamic(void* initialData, size_t size, const UniformDataMap& uniforms) :
		RHIUniformBufferDynamic(uniforms),
		m_Common(initialData, size)
	{
		DX11Logger.Info("DX11UniformBufferDynamic has been created.");
	}

	DX11UniformBufferDynamic::~DX11UniformBufferDynamic()
	{
		DX11Logger.Info("DX11UniformBufferDynamic has been destroyed.");
	}

	Result<void, RHIError> DX11UniformBufferDynamic::Bind(uint32 slot) const
	{
		return m_Common.Bind(slot);
	}

	const UniformData* DX11UniformBufferDynamic::GetUniformData(const String& name) const
	{
		auto it = GetUniformDataMap().find(name);
		if (it == GetUniformDataMap().end())
			return nullptr;
		return &it->second;
	}

	Result<void, RHIError> DX11UniformBufferDynamic::UpdateData() const
	{
		return m_Common.UpdateData();
	}

	bool DX11UniformBufferDynamic::SetUniformValue_Internal(const String& name, const void* value)
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

	void* DX11UniformBufferDynamic::GetUniformAddress(const String& name) const
	{
		auto it = GetUniformDataMap().find(name);
		if (it == GetUniformDataMap().end())
			return nullptr;

		return (uint8*)m_Common.Data + it->second.Offset;
	}
}

#endif // RHI_BUILD_DX11
