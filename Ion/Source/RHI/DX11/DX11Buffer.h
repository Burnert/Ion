#pragma once

#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/UniformBuffer.h"
#include "DX11.h"

namespace Ion
{
	class ION_API DX11VertexBuffer : public RHIVertexBuffer
	{
	public:
		DX11VertexBuffer(float* vertexAttributes, uint64 count);
		virtual ~DX11VertexBuffer() override;

		virtual void SetLayout(const TShared<RHIVertexLayout>& layout) override;
		virtual void SetLayoutShader(const TShared<RHIShader>& shader) override;

		void CreateDX11Layout(const TShared<class DX11Shader>& shader);

		virtual uint32 GetVertexCount() const override;

		static constexpr const char* GetSemanticName(const EVertexAttributeSemantic semantic)
		{
			switch (semantic)
			{
			case EVertexAttributeSemantic::Position:  return "POSITION";
			case EVertexAttributeSemantic::Normal:    return "NORMAL";
			case EVertexAttributeSemantic::TexCoord:  return "TEXCOORD";
			default:                                  return "";
			}
		}

	protected:
		virtual void Bind() const override;
		virtual void BindLayout() const override;
		virtual void Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_VertexCount;
		TShared<RHIVertexLayout> m_VertexLayout;
		TArray<D3D11_INPUT_ELEMENT_DESC> m_IEDArray;

		ID3D11Buffer* m_Buffer;
		ID3D11InputLayout* m_InputLayout;

		friend class DX11Renderer;
	};

	class ION_API DX11IndexBuffer : public RHIIndexBuffer
	{
	public:
		DX11IndexBuffer(uint32* indices, uint32 count);
		virtual ~DX11IndexBuffer() override;

		virtual uint32 GetIndexCount() const override;
		virtual uint32 GetTriangleCount() const override;

	protected:
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_Count;
		uint32 m_TriangleCount;

		ID3D11Buffer* m_Buffer;

		friend class DX11Renderer;
	};

	class _DX11UniformBufferCommon
	{
		_DX11UniformBufferCommon(void* initialData, size_t size);
		~_DX11UniformBufferCommon();

		void Bind(uint32 slot) const;
		void UpdateData() const;

		void* Data;
		size_t DataSize;
		ID3D11Buffer* Buffer;

		friend class DX11UniformBuffer;
		friend class DX11UniformBufferDynamic;
	};

	class ION_API DX11UniformBuffer : public RHIUniformBuffer
	{
	public:
		virtual void Bind(uint32 slot = 0) const override;

	protected:
		DX11UniformBuffer(void* initialData, size_t size);

		virtual void* GetDataPtr() const override;
		virtual void UpdateData() const override;

	private:
		_DX11UniformBufferCommon m_Common;

		friend class DX11Renderer;
		friend class RHIUniformBuffer;
	};

	class ION_API DX11UniformBufferDynamic : public RHIUniformBufferDynamic
	{
	protected:
		DX11UniformBufferDynamic(void* initialData, size_t size, const UniformDataMap& uniforms);

		virtual void Bind(uint32 slot = 0) const override;

		virtual const UniformData* GetUniformData(const String& name) const override;

	protected:
		virtual void UpdateData() const override;

		virtual bool SetUniformValue_Internal(const String& name, const void* value) override;
		virtual void* GetUniformAddress(const String& name) const override;

	private:
		_DX11UniformBufferCommon m_Common;

		friend class RHIUniformBufferDynamic;
	};
}
