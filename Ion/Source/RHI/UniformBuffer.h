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

#define _UNIFORM_TYPE_SIZE_HELPER(name, type) case EUniformType::name: return sizeof(type)
	constexpr size_t GetUniformTypeSize(EUniformType type)
	{
		switch (type)
		{
			_UNIFORM_TYPE_SIZE_HELPER(Float,     float);
			_UNIFORM_TYPE_SIZE_HELPER(Float2,    Vector2);
			_UNIFORM_TYPE_SIZE_HELPER(Float3,    Vector3);
			_UNIFORM_TYPE_SIZE_HELPER(Float4,    Vector4);
			_UNIFORM_TYPE_SIZE_HELPER(Int,       int32);
			_UNIFORM_TYPE_SIZE_HELPER(Int2,      IVector2);
			_UNIFORM_TYPE_SIZE_HELPER(Int3,      IVector3);
			_UNIFORM_TYPE_SIZE_HELPER(Int4,      IVector4);
			_UNIFORM_TYPE_SIZE_HELPER(UInt,      uint32);
			_UNIFORM_TYPE_SIZE_HELPER(UInt2,     UVector2);
			_UNIFORM_TYPE_SIZE_HELPER(UInt3,     UVector3);
			_UNIFORM_TYPE_SIZE_HELPER(UInt4,     UVector4);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix2,   Matrix2);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix2x3, Matrix2x3);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix2x4, Matrix2x4);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix3,   Matrix3);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix3x2, Matrix3x2);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix3x4, Matrix3x4);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix4,   Matrix4);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix4x2, Matrix4x2);
			_UNIFORM_TYPE_SIZE_HELPER(Matrix4x3, Matrix4x3);
		}
		return 0;
	}

#define _UNIFORM_GET_DEFAULT_HELPER(name, type) case EUniformType::name: *(type*)outValue = type(); break;
	inline void GetDefaultUniformValue(EUniformType type, void* outValue)
	{
		switch (type)
		{
			_UNIFORM_GET_DEFAULT_HELPER(Float,     float);
			_UNIFORM_GET_DEFAULT_HELPER(Float2,    Vector2);
			_UNIFORM_GET_DEFAULT_HELPER(Float3,    Vector3);
			_UNIFORM_GET_DEFAULT_HELPER(Float4,    Vector4);
			_UNIFORM_GET_DEFAULT_HELPER(Int,       int32);
			_UNIFORM_GET_DEFAULT_HELPER(Int2,      IVector2);
			_UNIFORM_GET_DEFAULT_HELPER(Int3,      IVector3);
			_UNIFORM_GET_DEFAULT_HELPER(Int4,      IVector4);
			_UNIFORM_GET_DEFAULT_HELPER(UInt,      uint32);
			_UNIFORM_GET_DEFAULT_HELPER(UInt2,     UVector2);
			_UNIFORM_GET_DEFAULT_HELPER(UInt3,     UVector3);
			_UNIFORM_GET_DEFAULT_HELPER(UInt4,     UVector4);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix2,   Matrix2);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix2x3, Matrix2x3);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix2x4, Matrix2x4);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix3,   Matrix3);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix3x2, Matrix3x2);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix3x4, Matrix3x4);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix4,   Matrix4);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix4x2, Matrix4x2);
			_UNIFORM_GET_DEFAULT_HELPER(Matrix4x3, Matrix4x3);
			default: ionassert(false);
		}
	}

	template<typename T>
	constexpr EUniformType TTypeToUniformTypeV = EUniformType::Null;

#define _UNIFORM_TYPE_TO_ENUM_HELPER(name, type) \
template<> constexpr EUniformType TTypeToUniformTypeV<type> = EUniformType::name

	_UNIFORM_TYPE_TO_ENUM_HELPER(Float,     float);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Float2,    Vector2);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Float3,    Vector3);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Float4,    Vector4);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Int,       int32);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Int2,      IVector2);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Int3,      IVector3);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Int4,      IVector4);
	_UNIFORM_TYPE_TO_ENUM_HELPER(UInt,      uint32);
	_UNIFORM_TYPE_TO_ENUM_HELPER(UInt2,     UVector2);
	_UNIFORM_TYPE_TO_ENUM_HELPER(UInt3,     UVector3);
	_UNIFORM_TYPE_TO_ENUM_HELPER(UInt4,     UVector4);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix2,   Matrix2);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix2x3, Matrix2x3);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix2x4, Matrix2x4);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix3,   Matrix3);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix3x2, Matrix3x2);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix3x4, Matrix3x4);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix4,   Matrix4);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix4x2, Matrix4x2);
	_UNIFORM_TYPE_TO_ENUM_HELPER(Matrix4x3, Matrix4x3);

	struct UniformData
	{
		String Name;
		uint32 Offset;
		EUniformType Type;
	};

	using UniformDataMap = THashMap<String, UniformData>;

	class RHIUniformBufferDynamic;

	class ION_API UniformBufferFactory
	{
	public:
		UniformBufferFactory();

		void Add(const String& name, EUniformType type);
		void Remove(const String& name);

		TShared<RHIUniformBufferDynamic> Construct();

	private:
		UniformDataMap m_Uniforms;
		uint32 m_CurrentSize;
	};

	class IRHIUniformBuffer
	{
	public:
		virtual void Bind(uint32 slot = 0) const = 0;

	protected:
		/**
		 * Copy the constants data from the CPU to the GPU.
		 * Accessible in Material
		 */
		virtual void UpdateData() const = 0;

		friend class Material;
	};

	class ION_API RHIUniformBuffer : public IRHIUniformBuffer
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
		RHIUniformBuffer() { };

		virtual void* GetDataPtr() const = 0;

	private:
		friend class Renderer;
		friend class OpenGLRenderer;
		friend class DX11Renderer;
	};

	class ION_API RHIUniformBufferDynamic : public IRHIUniformBuffer
	{
	public:
		static RHIUniformBufferDynamic* Create(void* initialData, size_t size, const UniformDataMap& uniforms);

		const UniformDataMap& GetUniformDataMap() const;

		template<typename T>
		bool SetUniformValue(const String& name, const T& value);

		virtual const UniformData* GetUniformData(const String& name) const = 0;

		bool HasUniform(const String& name) const;

	protected:
		RHIUniformBufferDynamic(const UniformDataMap& uniforms);

		virtual bool SetUniformValue_Internal(const String& name, const void* value) = 0;
		virtual void* GetUniformAddress(const String& name) const = 0;

	private:
		UniformDataMap m_UniformDataMap;

		friend class Renderer;
		friend class OpenGLRenderer;
		friend class DX11Renderer;
		friend class UniformBufferFactory;
	};

	inline const UniformDataMap& RHIUniformBufferDynamic::GetUniformDataMap() const
	{
		return m_UniformDataMap;
	}

	template<typename T>
	inline bool RHIUniformBufferDynamic::SetUniformValue(const String& name, const T& value)
	{
		static_assert(!TIsPointerV<T>);

		ionassert(HasUniform(name));
		ionassert(m_UniformDataMap.at(name).Type == TTypeToUniformTypeV<T>);

		return SetUniformValue_Internal(name, &value);
	}

	inline bool RHIUniformBufferDynamic::HasUniform(const String& name) const
	{
		return m_UniformDataMap.find(name) != m_UniformDataMap.end();
	}
}
