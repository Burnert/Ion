#pragma once

namespace Ion
{
	class ION_API UniformBuffer
	{
	public:
		static TShared<UniformBuffer> Create(void* data, size_t size);
		
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

		virtual void Bind() const = 0;

	protected:
		UniformBuffer() { }

	private:
		friend class Renderer;
	};
}
