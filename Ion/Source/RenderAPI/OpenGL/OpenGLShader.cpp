#include "IonPCH.h"

#include "OpenGLShader.h"

namespace Ion
{
	OpenGLShader::OpenGLShader() :
		m_ProgramID(0), 
		m_bCompiled(false)
	{ }

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_ProgramID);
		m_ProgramID = 0;
		CleanupDeleteShaders();
	}

	void OpenGLShader::AddShaderSource(EShaderType type, std::string source)
	{
		if (!m_bCompiled)
		{
			uint id;

			auto& it = m_Shaders.find(type);
			if (it == m_Shaders.end())
			{
				id = glCreateShader(ShaderTypeToGLShaderType(type));
			}
			else
			{
				id = (*it).second.ID;
			}

			const char* src = source.c_str();
			glShaderSource(id, 1, &src, 0);

			m_Shaders[type] = { id, type, source };
		}
	}

	bool OpenGLShader::Compile()
	{
		m_ProgramID = glCreateProgram();

		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glCompileShader(shader.ID);

			int bSuccess = 0;
			glGetShaderiv(shader.ID, GL_COMPILE_STATUS, &bSuccess);

			if (!bSuccess)
			{
				LOG_ERROR("Could not compile shader! ({0})", ShaderTypeToString(shader.Type));
				ASSERT(bSuccess);

				CleanupDeleteShaders();
				return false;
			}
		}

		m_bCompiled = true;

		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glAttachShader(m_ProgramID, shader.ID);
		}

		glLinkProgram(m_ProgramID);

		int bLinked = 0;
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &bLinked);

		if (bLinked)
		{
			// Shaders need to be detached after a successful link
			for (auto& entry : m_Shaders)
			{
				const SShaderInfo& shader = entry.second;

				glDetachShader(m_ProgramID, shader.ID);
			}
		}
		else
		{
			LOG_ERROR("Could not link shader program!");
			ASSERT(bLinked);

			glDeleteProgram(m_ProgramID);
			m_ProgramID = 0;
		}

		return bLinked;
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_ProgramID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::CleanupDeleteShaders()
	{
		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glDeleteShader(shader.ID);
		}

		m_Shaders.clear();
	}
}
