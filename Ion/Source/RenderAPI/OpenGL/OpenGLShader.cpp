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

	void OpenGLShader::SetUniform1f(const std::string& name, float value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform1fv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2f(const std::string& name, const FVector2& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform2fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform3f(const std::string& name, const FVector3& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform3fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform4f(const std::string& name, const FVector4& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform4fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform1i(const std::string& name, int value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform1iv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2i(const std::string& name, const IVector2& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform2iv(location, 1, (int*)&value);
	}

	void OpenGLShader::SetUniform3i(const std::string& name, const IVector3& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform3iv(location, 1, (int*)&value);
	}

	void OpenGLShader::SetUniform4i(const std::string& name, const IVector4& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform4iv(location, 1, (int*)&value);
	}

	void OpenGLShader::SetUniform1ui(const std::string& name, uint value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform1uiv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2ui(const std::string& name, const UVector2& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform2uiv(location, 1, (uint*)&value);
	}

	void OpenGLShader::SetUniform3ui(const std::string& name, const UVector3& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform3uiv(location, 1, (uint*)&value);
	}

	void OpenGLShader::SetUniform4ui(const std::string& name, const UVector4& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform4uiv(location, 1, (uint*)&value);
	}

	void OpenGLShader::SetUniformMatrix2f(const std::string& name, const FMatrix2& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix2x3f(const std::string& name, const FMatrix2x3& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix2x3fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix2x4f(const std::string& name, const FMatrix2x4& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix2x4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3f(const std::string& name, const FMatrix3& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3x2f(const std::string& name, const FMatrix3x2& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix3x2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3x4f(const std::string& name, const FMatrix3x4& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix3x4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4f(const std::string& name, const FMatrix4& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4x2f(const std::string& name, const FMatrix4x2& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix4x2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4x3f(const std::string& name, const FMatrix4x3& value) const
	{
		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix4x3fv(location, 1, false, (float*)&value);
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) const
	{
		int location;

		auto it = m_UniformCache.find(name);
		if (it == m_UniformCache.end())
		{
			location = glGetUniformLocation(m_ProgramID, name.c_str());
			m_UniformCache[name] = location;
		}
		else
		{
			location = m_UniformCache.at(name);
		}
		return location;
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
