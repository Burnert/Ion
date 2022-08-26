#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_DX10

#include "DX10Buffer.h"
#include "DX10Shader.h"

#pragma warning(disable:6387)

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
		DX10Logger.Info("DX10VertexBuffer has been created.");

		CreateBuffer(vertexAttributes, count)
			.Err([](Error& error) { DX10Logger.Critical("Cannot create a Vertex Buffer.\n{}", error.Message); })
			.Unwrap();
	}

	DX10VertexBuffer::~DX10VertexBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);
		COMRelease(m_InputLayout);

		DX10Logger.Info("DX10VertexBuffer has been destroyed.");
	}

	void DX10VertexBuffer::SetLayout(const TRef<RHIVertexLayout>& layout)
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

	Result<void, RHIError> DX10VertexBuffer::SetLayoutShader(const TRef<RHIShader>& shader)
	{
		return CreateDX10Layout(RefCast<DX10Shader>(shader));
	}

	Result<void, RHIError> DX10VertexBuffer::CreateDX10Layout(const TRef<DX10Shader>& shader)
	{
		TRACE_FUNCTION();

		ionassert(shader);
		ionassert(shader->IsCompiled());
		ionassert(m_VertexLayout);
		ionassert(!m_IEDArray.empty());

		ID3D10Device* device = DX10::GetDevice();

		ID3DBlob* blob = shader->GetVertexShaderByteCode();

		dxcall(device->CreateInputLayout(m_IEDArray.data(), (uint32)m_IEDArray.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &m_InputLayout),
			"Could not create Input Layout.");

		DX10Logger.Debug("DX10VertexBuffer Input Layout object has been created.");

		return Ok();
	}

	uint32 DX10VertexBuffer::GetVertexCount() const
	{
		// @TODO: 0...
		return 0;
	}

	Result<void, RHIError> DX10VertexBuffer::Bind() const
	{
		ionassert(m_VertexLayout, "Vertex Layout has not been set.");

		ID3D10Device* device = DX10::GetDevice();

		uint32 stride = m_VertexLayout->GetStride();
		uint32 offset = 0;
		dxcall(device->IASetVertexBuffers(0, 1, &m_Buffer, &stride, &offset));

		return Ok();
	}

	Result<void, RHIError> DX10VertexBuffer::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr));

		return Ok();
	}

	Result<void, RHIError> DX10VertexBuffer::BindLayout() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->IASetInputLayout(m_InputLayout));

		return Ok();
	}

	Result<void, RHIError> DX10VertexBuffer::CreateBuffer(float* vertexAttributes, uint64 count)
	{
		TRACE_FUNCTION();

		ionverify(vertexAttributes);
		ionassert(count <= std::numeric_limits<uint64>::max() / 4);
		ionassert(count * sizeof(float) <= std::numeric_limits<UINT>::max());

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
		DX10Logger.Debug("DX10VertexBuffer Buffer object has been created.");

		return Ok();
	}

	// -------------------------------------------------------------------
	// DX10IndexBuffer ---------------------------------------------------
	// -------------------------------------------------------------------

	DX10IndexBuffer::DX10IndexBuffer(uint32* indices, uint32 count) :
		m_Count(count),
		m_TriangleCount(count / 3),
		m_ID(0)
	{
		DX10Logger.Info("DX10IndexBuffer has been created.");

		CreateBuffer(indices, count)
			.Err([](Error& error) { DX10Logger.Critical("Cannot create an Index Buffer.\n{}", error.Message); })
			.Unwrap();
	}

	DX10IndexBuffer::~DX10IndexBuffer()
	{
		TRACE_FUNCTION();

		COMRelease(m_Buffer);

		DX10Logger.Info("DX10IndexBuffer has been destroyed.");
	}

	uint32 DX10IndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	uint32 DX10IndexBuffer::GetTriangleCount() const
	{
		return m_TriangleCount;
	}

	Result<void, RHIError> DX10IndexBuffer::Bind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->IASetIndexBuffer(m_Buffer, DXGI_FORMAT_R32_UINT, 0));

		return Ok();
	}

	Result<void, RHIError> DX10IndexBuffer::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));

		return Ok();
	}

	Result<void, RHIError> DX10IndexBuffer::CreateBuffer(uint32* indices, uint64 count)
	{
		TRACE_FUNCTION();

		ionverify(indices);

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

		DX10Logger.Debug("DX10IndexBuffer Buffer object has been created.");

		return Ok();
	}

	// -------------------------------------------------------------------
	// DX10UniformBuffer -------------------------------------------------
	// -------------------------------------------------------------------

	// Common Uniform Buffer -------------------------------------------------------

	_DX10UniformBufferCommon::_DX10UniformBufferCommon(void* initialData, size_t size) :
		DataSize(size),
		Buffer(nullptr)
	{
		DX10Logger.Debug("DX10UniformBufferCommon has been created.");

		CreateBuffer(initialData, size)
			.Err([](Error& error) { DX10Logger.Critical("Cannot create a Uniform Buffer.\n{}", error.Message); })
			.Unwrap();
	}

	_DX10UniformBufferCommon::~_DX10UniformBufferCommon()
	{
		TRACE_FUNCTION();

		COMRelease(Buffer);

		_aligned_free(Data);

		DX10Logger.Debug("DX10UniformBufferCommon has been destroyed.");
	}

	Result<void, RHIError> _DX10UniformBufferCommon::Bind(uint32 slot) const
	{
		ionassert(Buffer);

		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->VSSetConstantBuffers(slot, 1, &Buffer));
		dxcall(device->PSSetConstantBuffers(slot, 1, &Buffer));

		return Ok();
	}

	Result<void, RHIError> _DX10UniformBufferCommon::UpdateData() const
	{
		// Might become useful
		//TRACE_FUNCTION();

		ionassert(Buffer);

		void* pData = nullptr;
		dxcall(Buffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &pData));
		memcpy(pData, Data, DataSize);
		dxcall(Buffer->Unmap());

		return Ok();
	}

	Result<void, RHIError> _DX10UniformBufferCommon::CreateBuffer(void* initialData, size_t size)
	{
		TRACE_FUNCTION();

		ionverify(initialData);
		ionassert(size <= std::numeric_limits<UINT>::max());

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

		DX10Logger.Debug("DX10UniformBufferCommon Buffer object has been created.");

		return Ok();
	}

	// Specific Uniform Buffer -------------------------------------------------------

	DX10UniformBuffer::DX10UniformBuffer(void* initialData, size_t size) :
		m_Common(initialData, size)
	{
		DX10Logger.Info("DX10UniformBuffer has been created.");
	}

	DX10UniformBuffer::~DX10UniformBuffer()
	{
		DX10Logger.Info("DX10UniformBuffer has been destroyed.");
	}

	Result<void, RHIError> DX10UniformBuffer::Bind(uint32 slot) const
	{
		return m_Common.Bind(slot);
	}

	void* DX10UniformBuffer::GetDataPtr() const
	{
		return m_Common.Data;
	}

	Result<void, RHIError> DX10UniformBuffer::UpdateData() const
	{
		return m_Common.UpdateData();
	}

	DX10UniformBufferDynamic::DX10UniformBufferDynamic(void* initialData, size_t size, const UniformDataMap& uniforms) :
		RHIUniformBufferDynamic(uniforms),
		m_Common(initialData, size)
	{
		DX10Logger.Info("DX10UniformBufferDynamic has been created.");
	}

	DX10UniformBufferDynamic::~DX10UniformBufferDynamic()
	{
		DX10Logger.Info("DX10UniformBufferDynamic has been destroyed.");
	}

	Result<void, RHIError> DX10UniformBufferDynamic::Bind(uint32 slot) const
	{
		return m_Common.Bind(slot);
	}

	const UniformData* DX10UniformBufferDynamic::GetUniformData(const String& name) const
	{
		auto it = GetUniformDataMap().find(name);
		if (it == GetUniformDataMap().end())
			return nullptr;
		return &it->second;
	}

	Result<void, RHIError> DX10UniformBufferDynamic::UpdateData() const
	{
		return m_Common.UpdateData();
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

#endif // RHI_BUILD_DX10
