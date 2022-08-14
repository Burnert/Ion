#pragma once

#include "RHI/Shader.h"
#include "OpenGL.h"

namespace Ion
{
	struct OpenGLShaderInfo
	{
		uint32 ID;
		String Source;
		EShaderType Type;
	};

	class ION_API OpenGLShader : public RHIShader
	{
		friend class OpenGLRenderer;
	public:
		OpenGLShader();
		virtual ~OpenGLShader() override;

		virtual void AddShaderSource(EShaderType type, const String& source) override;

		virtual Result<void, ShaderCompilationError> Compile() override;
		virtual bool IsCompiled() override;

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
		THashMap<EShaderType, OpenGLShaderInfo> m_Shaders;

		mutable THashMap<String, int32> m_UniformCache;
	};
}
