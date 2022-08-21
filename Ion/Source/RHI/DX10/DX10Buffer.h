#pragma once

#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/UniformBuffer.h"
#include "DX10.h"

namespace Ion
{
	class ION_API DX10VertexBuffer : public RHIVertexBuffer
	{
	public:
		DX10VertexBuffer(float* vertexAttributes, uint64 count);
		virtual ~DX10VertexBuffer() override;

		virtual void SetLayout(const TShared<RHIVertexLayout>& layout) override;
		virtual Result<void, RHIError> SetLayoutShader(const TShared<RHIShader>& shader) override;

		Result<void, RHIError> CreateDX10Layout(const TShared<class DX10Shader>& shader);

		virtual uint32 GetVertexCount() const override;

	protected:
		virtual Result<void, RHIError> Bind() const override;
		virtual Result<void, RHIError> BindLayout() const override;
		virtual Result<void, RHIError> Unbind() const override;

	private:
		Result<void, RHIError> CreateBuffer(float* vertexAttributes, uint64 count);

	private:
		uint32 m_ID;
		uint32 m_VertexCount;
		TShared<RHIVertexLayout> m_VertexLayout;
		TArray<D3D10_INPUT_ELEMENT_DESC> m_IEDArray;

		ID3D10Buffer* m_Buffer;
		ID3D10InputLayout* m_InputLayout;

		friend class DX10Renderer;
	};

	class ION_API DX10IndexBuffer : public RHIIndexBuffer
	{
	public:
		DX10IndexBuffer(uint32* indices, uint32 count);
		virtual ~DX10IndexBuffer() override;

		virtual uint32 GetIndexCount() const override;
		virtual uint32 GetTriangleCount() const override;

	protected:
		virtual Result<void, RHIError> Bind() const override;
		virtual Result<void, RHIError> Unbind() const override;

	private:
		Result<void, RHIError> CreateBuffer(uint32* indices, uint64 count);

	private:
		uint32 m_ID;
		uint32 m_Count;
		uint32 m_TriangleCount;

		ID3D10Buffer* m_Buffer;

		friend class DX10Renderer;
	};

	class _DX10UniformBufferCommon
	{
		_DX10UniformBufferCommon(void* initialData, size_t size);
		~_DX10UniformBufferCommon();

		Result<void, RHIError> Bind(uint32 slot) const;
		Result<void, RHIError> UpdateData() const;

		Result<void, RHIError> CreateBuffer(void* initialData, size_t size);

		void* Data;
		size_t DataSize;
		ID3D10Buffer* Buffer;

		friend class DX10UniformBuffer;
		friend class DX10UniformBufferDynamic;
	};

	class ION_API DX10UniformBuffer : public RHIUniformBuffer
	{
	public:
		virtual Result<void, RHIError> Bind(uint32 slot = 0) const override;

	protected:
		DX10UniformBuffer(void* initialData, size_t size);

		virtual void* GetDataPtr() const override;
		virtual Result<void, RHIError> UpdateData() const override;

	private:
		_DX10UniformBufferCommon m_Common;

		friend class DX10Renderer;
		friend class RHIUniformBuffer;
	};

	class ION_API DX10UniformBufferDynamic : public RHIUniformBufferDynamic
	{
	protected:
		DX10UniformBufferDynamic(void* initialData, size_t size, const UniformDataMap& uniforms);

		virtual Result<void, RHIError> Bind(uint32 slot = 0) const override;

		virtual const UniformData* GetUniformData(const String& name) const override;

	protected:
		virtual Result<void, RHIError> UpdateData() const override;

		virtual bool SetUniformValue_Internal(const String& name, const void* value) override;
		virtual void* GetUniformAddress(const String& name) const override;

	private:
		_DX10UniformBufferCommon m_Common;

		friend class RHIUniformBufferDynamic;
	};
}
