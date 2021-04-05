#pragma once

namespace Ion
{
	enum class EShaderType
	{
		Vertex,
		Pixel,
	};

	class ION_API Shader
	{
	public:
		static TShared<Shader> Create(EShaderType shaderType, std::string shaderSource);

		virtual ~Shader() { }

		virtual bool Compile() const = 0;

	protected:
		Shader(EShaderType shaderType, std::string shaderSource);

	private:
		EShaderType m_Type;
		std::string m_ShaderSource;
	};

	class ION_API Program
	{
	public:
		static TShared<Program> Create();

		virtual ~Program() { }

		virtual void AttachShader(TShared<Shader> shader) = 0;

		virtual bool Link() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

	protected:
		std::vector<TShared<Shader>> m_AttachedShaders;
	};
}
