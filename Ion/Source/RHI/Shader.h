#pragma once

#include "RHICore.h"

namespace Ion
{
	enum class EShaderType : uint8
	{
		Unknown,
		Vertex,
		Pixel,
	};

	DEFINE_ERROR_TYPE(ShaderCompilationError);

	class ION_API RHIShader
	{
	public:
		static std::shared_ptr<RHIShader> Create();

		virtual ~RHIShader() { }

		virtual void AddShaderSource(EShaderType type, const String& source) = 0;

		virtual Result<void, RHIError, ShaderCompilationError> Compile() = 0;
		virtual bool IsCompiled() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

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

		friend class Renderer;
	};
}
