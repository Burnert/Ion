#pragma once

#include "RHI/Shader.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLShader : public Shader
	{
		friend class OpenGLRenderer;
	public:
		OpenGLShader();
		virtual ~OpenGLShader() override;

		virtual void AddShaderSource(EShaderType type, const String& source) override;

		virtual bool Compile() override;

		virtual bool HasUniform(const String& name) const override;

		virtual void SetUniform1f(const String& name, float value) const override;
		virtual void SetUniform2f(const String& name, const Vector2& value) const override;
		virtual void SetUniform3f(const String& name, const Vector3& value) const override;
		virtual void SetUniform4f(const String& name, const Vector4& value) const override;
		virtual void SetUniform1i(const String& name, int32 value) const override;
		virtual void SetUniform2i(const String& name, const IVector2& value) const override;
		virtual void SetUniform3i(const String& name, const IVector3& value) const override;
		virtual void SetUniform4i(const String& name, const IVector4& value) const override;
		virtual void SetUniform1ui(const String& name, uint32 value) const override;
		virtual void SetUniform2ui(const String& name, const UVector2& value) const override;
		virtual void SetUniform3ui(const String& name, const UVector3& value) const override;
		virtual void SetUniform4ui(const String& name, const UVector4& value) const override;
		virtual void SetUniformMatrix2f(const String& name, const Matrix2& value) const override;
		virtual void SetUniformMatrix2x3f(const String& name, const Matrix2x3& value) const override;
		virtual void SetUniformMatrix2x4f(const String& name, const Matrix2x4& value) const override;
		virtual void SetUniformMatrix3f(const String& name, const Matrix3& value) const override;
		virtual void SetUniformMatrix3x2f(const String& name, const Matrix3x2& value) const override;
		virtual void SetUniformMatrix3x4f(const String& name, const Matrix3x4& value) const override;
		virtual void SetUniformMatrix4f(const String& name, const Matrix4& value) const override;
		virtual void SetUniformMatrix4x2f(const String& name, const Matrix4x2& value) const override;
		virtual void SetUniformMatrix4x3f(const String& name, const Matrix4x3& value) const override;

		int32 GetUniformLocation(const String& name) const;

		FORCEINLINE static constexpr uint32 ShaderTypeToGLShaderType(EShaderType type)
		{
			switch (type)
			{
			case EShaderType::Vertex:    return GL_VERTEX_SHADER;
			case EShaderType::Pixel:     return GL_FRAGMENT_SHADER;
			default:                     return 0;
			}
		}

	protected:
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		void CleanupDeleteShaders();
		void InvalidateUniformCache();

	private:
		uint32 m_ProgramID;
		bool m_bCompiled;
		THashMap<EShaderType, ShaderInfo> m_Shaders;

		mutable THashMap<String, int32> m_UniformCache;
	};
}
