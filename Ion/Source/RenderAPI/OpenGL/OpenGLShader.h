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
	};
}
