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

	class UniformBuffer;

	class ION_API UniformBufferFactory
	{
	public:
		void Add(const String& name, EUniformType type);
		void Remove(const String& name);

		void Construct(TShared<UniformBuffer>& outUniformBuffer);
		void Construct(UniformBuffer*& outUniformBuffer);

	private:
		UniformDataMap m_Uniforms;
	};

	class ION_API UniformBuffer
	{
	public:
		static UniformBuffer* Create(void* initialData, size_t size);

		template<typename T>
		static UniformBuffer* Create(T& initialData)
		{
			return Create(&initialData, sizeof(T));
		}

		virtual ~UniformBuffer() { }

		virtual void SetFloat(const String& name, float value) const = 0;
		virtual void SetFloat2(const String& name, const Vector2& value) const = 0;
		virtual void SetFloat3(const String& name, const Vector3& value) const = 0;
		virtual void SetFloat4(const String& name, const Vector4& value) const = 0;
		virtual void SetInt(const String& name, int32 value) const = 0;
		virtual void SetInt2(const String& name, const IVector2& value) const = 0;
		virtual void SetInt3(const String& name, const IVector3& value) const = 0;
		virtual void SetInt4(const String& name, const IVector4& value) const = 0;
		virtual void SetUInt(const String& name, uint32 value) const = 0;
		virtual void SetUInt2(const String& name, const UVector2& value) const = 0;
		virtual void SetUInt3(const String& name, const UVector3& value) const = 0;
		virtual void SetUInt4(const String& name, const UVector4& value) const = 0;
		virtual void SetMatrix2(const String& name, const Matrix2& value) const = 0;
		virtual void SetMatrix2x3(const String& name, const Matrix2x3& value) const = 0;
		virtual void SetMatrix2x4(const String& name, const Matrix2x4& value) const = 0;
		virtual void SetMatrix3(const String& name, const Matrix3& value) const = 0;
		virtual void SetMatrix3x2(const String& name, const Matrix3x2& value) const = 0;
		virtual void SetMatrix3x4(const String& name, const Matrix3x4& value) const = 0;
		virtual void SetMatrix4(const String& name, const Matrix4& value) const = 0;
		virtual void SetMatrix4x2(const String& name, const Matrix4x2& value) const = 0;
		virtual void SetMatrix4x3(const String& name, const Matrix4x3& value) const = 0;

		virtual void Bind(uint32 slot = 0) const = 0;

		static constexpr uint32 SizeOfUniformType(EUniformType type)
		{
			switch (type)
			{
			case Ion::EUniformType::Null:       return 0;
			case Ion::EUniformType::Float:      return sizeof(float);
			case Ion::EUniformType::Float2:     return sizeof(Vector2);
			case Ion::EUniformType::Float3:     return sizeof(Vector3);
			case Ion::EUniformType::Float4:     return sizeof(Vector4);
			case Ion::EUniformType::Int:        return sizeof(int32);
			case Ion::EUniformType::Int2:       return sizeof(IVector2);
			case Ion::EUniformType::Int3:       return sizeof(IVector3);
			case Ion::EUniformType::Int4:       return sizeof(IVector4);
			case Ion::EUniformType::UInt:       return sizeof(uint32);
			case Ion::EUniformType::UInt2:      return sizeof(UVector2);
			case Ion::EUniformType::UInt3:      return sizeof(UVector3);
			case Ion::EUniformType::UInt4:      return sizeof(UVector4);
			case Ion::EUniformType::Matrix2:    return sizeof(Matrix2);
			case Ion::EUniformType::Matrix2x3:  return sizeof(Matrix2x3);
			case Ion::EUniformType::Matrix2x4:  return sizeof(Matrix2x4);
			case Ion::EUniformType::Matrix3:    return sizeof(Matrix3);
			case Ion::EUniformType::Matrix3x2:  return sizeof(Matrix3x2);
			case Ion::EUniformType::Matrix3x4:  return sizeof(Matrix3x4);
			case Ion::EUniformType::Matrix4:    return sizeof(Matrix4);
			case Ion::EUniformType::Matrix4x2:  return sizeof(Matrix4x2);
			case Ion::EUniformType::Matrix4x3:  return sizeof(Matrix4x3);
			default:                            return 0;
			}
		}

		// Make sure to call this with the right type.
		template<typename T>
		T* Data() const
		{
			return (T*)GetDataPtr();
		}

	protected:
		static UniformBuffer* Create(void* data, size_t size, const UniformDataMap& uniforms);

		UniformBuffer() { };
		UniformBuffer(const UniformDataMap& uniforms);

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
