#pragma once

#include "Renderer/Shader.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader();
		virtual ~OpenGLShader() override;

		virtual void AddShaderSource(EShaderType type, std::string source) override;

		virtual bool Compile() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetUniform1f(const std::string& name, float value) const override;
		virtual void SetUniform2f(const std::string& name, const FVector2& value) const override;
		virtual void SetUniform3f(const std::string& name, const FVector3& value) const override;
		virtual void SetUniform4f(const std::string& name, const FVector4& value) const override;
		virtual void SetUniform1i(const std::string& name, int value) const override;
		virtual void SetUniform2i(const std::string& name, const IVector2& value) const override;
		virtual void SetUniform3i(const std::string& name, const IVector3& value) const override;
		virtual void SetUniform4i(const std::string& name, const IVector4& value) const override;
		virtual void SetUniform1ui(const std::string& name, uint value) const override;
		virtual void SetUniform2ui(const std::string& name, const UVector2& value) const override;
		virtual void SetUniform3ui(const std::string& name, const UVector3& value) const override;
		virtual void SetUniform4ui(const std::string& name, const UVector4& value) const override;
		virtual void SetUniformMatrix2f(const std::string& name, const FMatrix2& value) const override;
		virtual void SetUniformMatrix2x3f(const std::string& name, const FMatrix2x3& value) const override;
		virtual void SetUniformMatrix2x4f(const std::string& name, const FMatrix2x4& value) const override;
		virtual void SetUniformMatrix3f(const std::string& name, const FMatrix3& value) const override;
		virtual void SetUniformMatrix3x2f(const std::string& name, const FMatrix3x2& value) const override;
		virtual void SetUniformMatrix3x4f(const std::string& name, const FMatrix3x4& value) const override;
		virtual void SetUniformMatrix4f(const std::string& name, const FMatrix4& value) const override;
		virtual void SetUniformMatrix4x2f(const std::string& name, const FMatrix4x2& value) const override;
		virtual void SetUniformMatrix4x3f(const std::string& name, const FMatrix4x3& value) const override;

		int GetUniformLocation(const std::string& name) const;

		FORCEINLINE static constexpr uint ShaderTypeToGLShaderType(EShaderType type)
		{
			switch (type)
			{
			case EShaderType::Vertex:    return GL_VERTEX_SHADER;
			case EShaderType::Pixel:     return GL_FRAGMENT_SHADER;
			default:                     return 0;
			}
		}

	private:
		void CleanupDeleteShaders();

	private:
		uint m_ProgramID;
		bool m_bCompiled;
		std::unordered_map<EShaderType, SShaderInfo> m_Shaders;

		mutable std::unordered_map<std::string, int> m_UniformCache;
	};
}
