#pragma once

namespace Ion
{
	enum class EShaderType
	{
		Vertex,
		Fragment,
	};

	class ION_API Shader
	{
	public:
		static Shared<Shader> Create(EShaderType shaderType, std::string shaderSource);

		virtual ~Shader() { };

		virtual bool Compile() = 0;

	protected:
		Shader(EShaderType shaderType, std::string shaderSource);

	private:
		EShaderType m_Type;
		std::string m_ShaderSource;
	};

	class ION_API Program
	{
	public:
		static Shared<Program> Create();

		virtual ~Program() { };

		virtual void AttachShader(Shared<Shader> shader) = 0;

		virtual bool Link() = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

	protected:
		std::vector<Shared<Shader>> m_AttachedShaders;
	};
}
