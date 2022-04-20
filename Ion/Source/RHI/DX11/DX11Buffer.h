#pragma once

#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/UniformBuffer.h"
#include "DX11.h"

namespace Ion
{
	class ION_API DX11VertexBuffer : public VertexBuffer
	{
	public:
		DX11VertexBuffer(float* vertexAttributes, uint64 count);
		virtual ~DX11VertexBuffer() override;

		virtual void SetLayout(const TShared<VertexLayout>& layout) override;
		virtual void SetLayoutShader(const TShared<Shader>& shader) override;

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

		struct VertexAttributeFormat
		{
			EVertexAttributeType Type;
			uint8 ElementCount;
		};

		static constexpr DXGI_FORMAT VertexAttributeToDXGIFormat(const VertexAttributeFormat attribute)
		{
			switch (attribute.ElementCount)
			{
			case 1:
			{
				switch (attribute.Type)
				{
				case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Byte:           return DXGI_FORMAT_R8_SINT;
				case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_R8_UINT;
				case EVertexAttributeType::Short:          return DXGI_FORMAT_R16_SINT;
				case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_R16_UINT;
				case EVertexAttributeType::Int:            return DXGI_FORMAT_R32_SINT;
				case EVertexAttributeType::UnsignedInt:    return DXGI_FORMAT_R32_UINT;
				case EVertexAttributeType::Float16:        return DXGI_FORMAT_R16_FLOAT;
				case EVertexAttributeType::Float: 		   return DXGI_FORMAT_R32_FLOAT;
				case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
				}
			}
			case 2:
			{
				switch (attribute.Type)
				{
				case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Byte:           return DXGI_FORMAT_R8G8_SINT;
				case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_R8G8_UINT;
				case EVertexAttributeType::Short:          return DXGI_FORMAT_R16G16_SINT;
				case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_R16G16_UINT;
				case EVertexAttributeType::Int:            return DXGI_FORMAT_R32G32_SINT;
				case EVertexAttributeType::UnsignedInt:    return DXGI_FORMAT_R32G32_UINT;
				case EVertexAttributeType::Float16:        return DXGI_FORMAT_R16G16_FLOAT;
				case EVertexAttributeType::Float:          return DXGI_FORMAT_R32G32_FLOAT;
				case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
				}
			}
			case 3:
			{
				switch (attribute.Type)
				{
				case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Byte:           return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Short:          return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Int:            return DXGI_FORMAT_R32G32B32_SINT;
				case EVertexAttributeType::UnsignedInt:    return DXGI_FORMAT_R32G32B32_UINT;
				case EVertexAttributeType::Float16:        return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Float:          return DXGI_FORMAT_R32G32B32_FLOAT;
				case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
				}
			}
			case 4:
			{
				switch (attribute.Type)
				{
				case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
				case EVertexAttributeType::Byte:           return DXGI_FORMAT_R8G8B8A8_SINT;
				case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_R8G8B8A8_UINT;
				case EVertexAttributeType::Short:          return DXGI_FORMAT_R16G16B16A16_SINT;
				case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_R16G16B16A16_UINT;
				case EVertexAttributeType::Int:            return DXGI_FORMAT_R32G32B32A32_SINT;
				case EVertexAttributeType::UnsignedInt:	   return DXGI_FORMAT_R32G32B32A32_UINT;
				case EVertexAttributeType::Float16:        return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case EVertexAttributeType::Float:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
				}
			}
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

	protected:
		virtual void Bind() const override;
		virtual void BindLayout() const override;
		virtual void Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_VertexCount;
		TShared<VertexLayout> m_VertexLayout;
		TArray<D3D11_INPUT_ELEMENT_DESC> m_IEDArray;

		ID3D11Buffer* m_Buffer;
		ID3D11InputLayout* m_InputLayout;

		friend class DX11Renderer;
	};

	class ION_API DX11IndexBuffer : public IndexBuffer
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

	class ION_API DX11UniformBuffer : public UniformBuffer
	{
	public:
		virtual ~DX11UniformBuffer() override;

		virtual void Bind(uint32 slot = 0) const override;

	protected:
		DX11UniformBuffer(void* initialData, size_t size);
		DX11UniformBuffer(void* data, size_t size, const UniformDataMap& uniforms);

		virtual void* GetDataPtr() const override;
		virtual void UpdateData() const override;

	private:
		void* m_Data;
		size_t m_DataSize;
		ID3D11Buffer* m_Buffer;

		friend class DX11Renderer;
		friend class UniformBuffer;
	};
}
