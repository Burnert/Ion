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
