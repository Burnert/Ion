#include "IonPCH.h"

#include "OpenGLShader.h"

namespace Ion
{
	// ------------------------------------------
	// Shader 

	OpenGLShader::OpenGLShader(EShaderType shaderType, std::string shaderSource)
		: Shader(shaderType, shaderSource)
	{
		m_ID = glCreateShader(ShaderTypeToGLShaderType(shaderType));

		const char* source = shaderSource.c_str();
		glShaderSource(m_ID, 1, &source, 0);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteShader(m_ID);
	}

	bool OpenGLShader::Compile()
	{
		glCompileShader(m_ID);

		int bSuccess = 0;
		glGetShaderiv(m_ID, GL_COMPILE_STATUS, &bSuccess);

		return bSuccess;
	}

	// ------------------------------------------
	// Program

	OpenGLProgram::OpenGLProgram()
	{
		m_ID = glCreateProgram();
	}

	OpenGLProgram::~OpenGLProgram()
	{
		glDeleteProgram(m_ID);
	}

	void OpenGLProgram::AttachShader(Shared<Shader> shader)
	{
		uint shaderId = std::static_pointer_cast<OpenGLShader>(shader)->m_ID;
		glAttachShader(m_ID, shaderId);
		m_AttachedShaders.push_back(shader);
	}

	bool OpenGLProgram::Link()
	{
		glLinkProgram(m_ID);

		int bLinked = 0;
		glGetProgramiv(m_ID, GL_LINK_STATUS, &bLinked);

		if (bLinked)
		{
			// Shaders need to be detached after a successful link
			for (Shared<Shader> shader : m_AttachedShaders)
			{
				uint shaderId = std::static_pointer_cast<OpenGLShader>(shader)->m_ID;
				glDetachShader(m_ID, shaderId);
			}
			m_AttachedShaders.clear();
		}

		return bLinked;
	}

	void OpenGLProgram::Bind()
	{
		glUseProgram(m_ID);
	}

	void OpenGLProgram::Unbind()
	{
		glUseProgram(0);
	}
}
