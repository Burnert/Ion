#pragma once

#include "VertexAttribute.h"

namespace Ion
{
	enum class EUniformType : uint8
	{
		Null = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4,
		UInt,
		UInt2,
		UInt3,
		UInt4,
		Matrix2,
		Matrix2x3,
		Matrix2x4,
		Matrix3,
		Matrix3x2,
		Matrix3x4,
		Matrix4,
		Matrix4x2,
		Matrix4x3
	};

	struct UniformData
	{
		void* Location;
		String Name;
		EUniformType Type;
	};

	using UniformDataMap = THashMap<String, UniformData>;

	class RHIUniformBuffer;

	class ION_API UniformBufferFactory
	{
	public:
		void Add(const String& name, EUniformType type);
		void Remove(const String& name);

		void Construct(TShared<RHIUniformBuffer>& outUniformBuffer);
		void Construct(RHIUniformBuffer*& outUniformBuffer);

	private:
		UniformDataMap m_Uniforms;
	};

	class ION_API RHIUniformBuffer
	{
	public:
		static RHIUniformBuffer* Create(void* initialData, size_t size);

		/* Creates a UniformBuffer with specified struct data. */
		template<typename T>
		static RHIUniformBuffer* Create(T& initialData)
		{
			return Create(&initialData, sizeof(T));
		}

		/* Creates a UniformBuffer with a specified struct type. */
		template<typename T>
		static RHIUniformBuffer* Create()
		{
			T initialData { };
			return Create(&initialData, sizeof(T));
		}

		virtual ~RHIUniformBuffer() { }

		virtual void Bind(uint32 slot = 0) const = 0;

		static constexpr uint32 SizeOfUniformType(EUniformType type)
		{
			switch (type)
			{
			case EUniformType::Null:       return 0;
			case EUniformType::Float:      return sizeof(float);
			case EUniformType::Float2:     return sizeof(Vector2);
			case EUniformType::Float3:     return sizeof(Vector3);
			case EUniformType::Float4:     return sizeof(Vector4);
			case EUniformType::Int:        return sizeof(int32);
			case EUniformType::Int2:       return sizeof(IVector2);
			case EUniformType::Int3:       return sizeof(IVector3);
			case EUniformType::Int4:       return sizeof(IVector4);
			case EUniformType::UInt:       return sizeof(uint32);
			case EUniformType::UInt2:      return sizeof(UVector2);
			case EUniformType::UInt3:      return sizeof(UVector3);
			case EUniformType::UInt4:      return sizeof(UVector4);
			case EUniformType::Matrix2:    return sizeof(Matrix2);
			case EUniformType::Matrix2x3:  return sizeof(Matrix2x3);
			case EUniformType::Matrix2x4:  return sizeof(Matrix2x4);
			case EUniformType::Matrix3:    return sizeof(Matrix3);
			case EUniformType::Matrix3x2:  return sizeof(Matrix3x2);
			case EUniformType::Matrix3x4:  return sizeof(Matrix3x4);
			case EUniformType::Matrix4:    return sizeof(Matrix4);
			case EUniformType::Matrix4x2:  return sizeof(Matrix4x2);
			case EUniformType::Matrix4x3:  return sizeof(Matrix4x3);
			default:                       return 0;
			}
		}

		// Returns a pointer to the uniform buffer data on the CPU side.
		// Make sure to call this with the right type.
		template<typename T>
		T* Data() const
		{
			return (T*)GetDataPtr();
		}

		// Returns a reference to the uniform buffer data on the CPU side.
		// Make sure to call this with the right type.
		template<typename T>
		T& DataRef() const
		{
			return *(T*)GetDataPtr();
		}

	protected:
		static RHIUniformBuffer* Create(void* data, size_t size, const UniformDataMap& uniforms);

		RHIUniformBuffer() { };
		RHIUniformBuffer(const UniformDataMap& uniforms);

		virtual void* GetDataPtr() const = 0;
		virtual void UpdateData() const = 0;

	private:
		// @TODO: This doesn't need to be here if it doesn't need to be here
		const TUnique<const UniformDataMap> m_Uniforms;

		friend class Renderer;
		friend class OpenGLRenderer;
		friend class DX11Renderer;
		friend class UniformBufferFactory;
	};
}
