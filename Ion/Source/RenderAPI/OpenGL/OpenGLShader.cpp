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
		TRACE_FUNCTION();

		glDeleteProgram(m_ProgramID);
		m_ProgramID = 0;
		CleanupDeleteShaders();
	}

	void OpenGLShader::AddShaderSource(EShaderType type, std::string source)
	{
		TRACE_FUNCTION();

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
		TRACE_FUNCTION();

		m_ProgramID = glCreateProgram();

		TRACE_BEGIN(0, "OpenGLShader - Shader Compilation");
		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glCompileShader(shader.ID);

			int bCompiled = 0;
			glGetShaderiv(shader.ID, GL_COMPILE_STATUS, &bCompiled);

			ionexcept(bCompiled, "Shader compilation failure! (%s)", ShaderTypeToString(shader.Type))
			{
				LOG_ERROR("Could not compile shader! ({0})", ShaderTypeToString(shader.Type));
				CleanupDeleteShaders();
				return false;
			}
		}
		TRACE_END(0);

		m_bCompiled = true;

		TRACE_BEGIN(1, "OpenGLShader - Shader Attachment");
		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glAttachShader(m_ProgramID, shader.ID);
		}
		TRACE_END(1);

		TRACE_BEGIN(2, "OpenGLShader - Shader Linking");
		glLinkProgram(m_ProgramID);
		TRACE_END(2);

		int bLinked = 0;
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &bLinked);

		ionexcept(bLinked, "Shader linkage failure!")
		{
			LOG_ERROR("Could not link shader program!");
			glDeleteProgram(m_ProgramID);
			m_ProgramID = 0;

			return false;
		}

		TRACE_BEGIN(3, "OpenGLShader - Shader Detachment");
		// Shaders need to be detached after a successful link
		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glDetachShader(m_ProgramID, shader.ID);
		}
		TRACE_END(3);

		return true;
	}

	bool OpenGLShader::HasUniform(const std::string& name) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		return location != -1;
	}

	void OpenGLShader::SetUniform1f(const std::string& name, float value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform1fv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2f(const std::string& name, const FVector2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform2fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform3f(const std::string& name, const FVector3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform3fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform4f(const std::string& name, const FVector4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform4fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform1i(const std::string& name, int value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform1iv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2i(const std::string& name, const IVector2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform2iv(location, 1, (int*)&value);
	}

	void OpenGLShader::SetUniform3i(const std::string& name, const IVector3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform3iv(location, 1, (int*)&value);
	}

	void OpenGLShader::SetUniform4i(const std::string& name, const IVector4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform4iv(location, 1, (int*)&value);
	}

	void OpenGLShader::SetUniform1ui(const std::string& name, uint value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform1uiv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2ui(const std::string& name, const UVector2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform2uiv(location, 1, (uint*)&value);
	}

	void OpenGLShader::SetUniform3ui(const std::string& name, const UVector3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform3uiv(location, 1, (uint*)&value);
	}

	void OpenGLShader::SetUniform4ui(const std::string& name, const UVector4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniform4uiv(location, 1, (uint*)&value);
	}

	void OpenGLShader::SetUniformMatrix2f(const std::string& name, const FMatrix2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix2x3f(const std::string& name, const FMatrix2x3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix2x3fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix2x4f(const std::string& name, const FMatrix2x4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix2x4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3f(const std::string& name, const FMatrix3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3x2f(const std::string& name, const FMatrix3x2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix3x2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3x4f(const std::string& name, const FMatrix3x4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix3x4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4f(const std::string& name, const FMatrix4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4x2f(const std::string& name, const FMatrix4x2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix4x2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4x3f(const std::string& name, const FMatrix4x3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int location = GetUniformLocation(name);
		glUniformMatrix4x3fv(location, 1, false, (float*)&value);
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) const
	{
		TRACE_FUNCTION();

		int location;

		auto it = m_UniformCache.find(name);
		if (it == m_UniformCache.end())
		{
			TRACE_BEGIN(0, "OpenGLShader - Uniform cache miss - glGetUniformLocation");
			location = glGetUniformLocation(m_ProgramID, name.c_str());
			m_UniformCache[name] = location;
			TRACE_END(0);
		}
		else
		{
			location = m_UniformCache.at(name);
		}
		return location;
	}

	void OpenGLShader::Bind() const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
	}

	void OpenGLShader::Unbind() const
	{
		TRACE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::CleanupDeleteShaders()
	{
		TRACE_FUNCTION();

		for (auto& entry : m_Shaders)
		{
			const SShaderInfo& shader = entry.second;

			glDeleteShader(shader.ID);
		}

		m_Shaders.clear();
	}
}
