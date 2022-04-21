#pragma once

namespace Ion
{
	enum class EShaderType : uint8
	{
		Unknown,
		Vertex,
		Pixel,
	};

	struct ShaderInfo
	{
		uint32 ID;
		String Source;
		EShaderType Type;
	};

	class ION_API RHIShader
	{
	public:
		static TShared<RHIShader> Create();

		virtual ~RHIShader() { }

		virtual void AddShaderSource(EShaderType type, const String& source) = 0;

		virtual bool Compile() = 0;

		virtual bool HasUniform(const String& name) const = 0;

		virtual void SetUniform1f(const String& name, float value) const = 0;
		virtual void SetUniform2f(const String& name, const Vector2& value) const = 0;
		virtual void SetUniform3f(const String& name, const Vector3& value) const = 0;
		virtual void SetUniform4f(const String& name, const Vector4& value) const = 0;
		virtual void SetUniform1i(const String& name, int32 value) const = 0;
		virtual void SetUniform2i(const String& name, const IVector2& value) const = 0;
		virtual void SetUniform3i(const String& name, const IVector3& value) const = 0;
		virtual void SetUniform4i(const String& name, const IVector4& value) const = 0;
		virtual void SetUniform1ui(const String& name, uint32 value) const = 0;
		virtual void SetUniform2ui(const String& name, const UVector2& value) const = 0;
		virtual void SetUniform3ui(const String& name, const UVector3& value) const = 0;
		virtual void SetUniform4ui(const String& name, const UVector4& value) const = 0;
		virtual void SetUniformMatrix2f(const String& name, const Matrix2& value) const = 0;
		virtual void SetUniformMatrix2x3f(const String& name, const Matrix2x3& value) const = 0;
		virtual void SetUniformMatrix2x4f(const String& name, const Matrix2x4& value) const = 0;
		virtual void SetUniformMatrix3f(const String& name, const Matrix3& value) const = 0;
		virtual void SetUniformMatrix3x2f(const String& name, const Matrix3x2& value) const = 0;
		virtual void SetUniformMatrix3x4f(const String& name, const Matrix3x4& value) const = 0;
		virtual void SetUniformMatrix4f(const String& name, const Matrix4& value) const = 0;
		virtual void SetUniformMatrix4x2f(const String& name, const Matrix4x2& value) const = 0;
		virtual void SetUniformMatrix4x3f(const String& name, const Matrix4x3& value) const = 0;

		FORCEINLINE static constexpr const char* ShaderTypeToString(EShaderType type)
		{
			switch (type)
			{
			case EShaderType::Vertex:  return "Vertex Shader";
			case EShaderType::Pixel:   return "Pixel Shader";
			default:                   return "Unknown Shader";
			}
		}

	protected:
		RHIShader() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		friend class Renderer;
	};
}
