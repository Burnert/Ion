#pragma once

#include "Renderer/Shader.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLShader : public Shader
	{
		friend class OpenGLProgram;
	public:
		OpenGLShader(EShaderType shaderType, std::string shaderSource);
		virtual ~OpenGLShader() override;

		virtual bool Compile() const override;

		static constexpr FORCEINLINE uint ShaderTypeToGLShaderType(EShaderType type)
		{
			switch (type)
			{
			case EShaderType::Vertex:    return GL_VERTEX_SHADER;
			case EShaderType::Fragment:  return GL_FRAGMENT_SHADER;
			default:                     return 0;
			}
		}

	private:
		uint m_ID;
	};

	class ION_API OpenGLProgram : public Program
	{
	public:
		OpenGLProgram();
		virtual ~OpenGLProgram() override;

		virtual void AttachShader(TShared<Shader> shader) override;

		virtual bool Link() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint m_ID;
	};
}
