#pragma once

namespace Ion
{
	enum class EShaderType : ubyte
	{
		Unknown,
		Vertex,
		Pixel,
	};

	struct SShaderInfo
	{
		uint ID;
		EShaderType Type;
		std::string Source;
	};

	class ION_API Shader
	{
	public:
		static TShared<Shader> Create();

		virtual ~Shader() { }

		virtual void AddShaderSource(EShaderType type, std::string source) = 0;

		virtual bool Compile() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetUniform1f(const std::string& name, float value) const = 0;
		virtual void SetUniform2f(const std::string& name, const FVector2& value) const = 0;
		virtual void SetUniform3f(const std::string& name, const FVector3& value) const = 0;
		virtual void SetUniform4f(const std::string& name, const FVector4& value) const = 0;
		virtual void SetUniform1i(const std::string& name, int value) const = 0;
		virtual void SetUniform2i(const std::string& name, const IVector2& value) const = 0;
		virtual void SetUniform3i(const std::string& name, const IVector3& value) const = 0;
		virtual void SetUniform4i(const std::string& name, const IVector4& value) const = 0;
		virtual void SetUniform1ui(const std::string& name, uint value) const = 0;
		virtual void SetUniform2ui(const std::string& name, const UVector2& value) const = 0;
		virtual void SetUniform3ui(const std::string& name, const UVector3& value) const = 0;
		virtual void SetUniform4ui(const std::string& name, const UVector4& value) const = 0;
		virtual void SetUniformMatrix2f(const std::string& name, const FMatrix2& value) const = 0;
		virtual void SetUniformMatrix2x3f(const std::string& name, const FMatrix2x3& value) const = 0;
		virtual void SetUniformMatrix2x4f(const std::string& name, const FMatrix2x4& value) const = 0;
		virtual void SetUniformMatrix3f(const std::string& name, const FMatrix3& value) const = 0;
		virtual void SetUniformMatrix3x2f(const std::string& name, const FMatrix3x2& value) const = 0;
		virtual void SetUniformMatrix3x4f(const std::string& name, const FMatrix3x4& value) const = 0;
		virtual void SetUniformMatrix4f(const std::string& name, const FMatrix4& value) const = 0;
		virtual void SetUniformMatrix4x2f(const std::string& name, const FMatrix4x2& value) const = 0;
		virtual void SetUniformMatrix4x3f(const std::string& name, const FMatrix4x3& value) const = 0;

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
		Shader() { }
	};
}
