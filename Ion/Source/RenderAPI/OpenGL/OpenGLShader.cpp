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

	void OpenGLShader::AddShaderSource(EShaderType type, String source)
	{
		TRACE_FUNCTION();

		if (!m_bCompiled)
		{
			uint32 id;

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

			m_Shaders[type] = { id, source, type };
		}
	}

	bool OpenGLShader::Compile()
	{
		TRACE_FUNCTION();

		m_ProgramID = glCreateProgram();

		TRACE_BEGIN(0, "OpenGLShader - Shader Compilation");
		for (auto& entry : m_Shaders)
		{
			const ShaderInfo& shader = entry.second;

			glCompileShader(shader.ID);

			int32 bCompiled = 0;
			glGetShaderiv(shader.ID, GL_COMPILE_STATUS, &bCompiled);

			ionexcept(bCompiled, "Shader compilation failure! (%s)", ShaderTypeToString(shader.Type))
			{
				LOG_ERROR("Could not compile shader! ({0})", ShaderTypeToString(shader.Type));

				int32 logLength = 0;
				glGetShaderiv(shader.ID, GL_INFO_LOG_LENGTH, &logLength);

				char* message = (char*)_malloca(logLength);
				glGetShaderInfoLog(shader.ID, logLength, &logLength, message);

				LOG_TRACE("Shader compilation log: \n{0}", message);

				CleanupDeleteShaders();
				return false;
			}
		}
		TRACE_END(0);

		m_bCompiled = true;

		TRACE_BEGIN(1, "OpenGLShader - Shader Attachment");
		for (auto& entry : m_Shaders)
		{
			const ShaderInfo& shader = entry.second;

			glAttachShader(m_ProgramID, shader.ID);
		}
		TRACE_END(1);

		TRACE_BEGIN(2, "OpenGLShader - Shader Linking");
		glLinkProgram(m_ProgramID);
		TRACE_END(2);

		int32 bLinked = 0;
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
			const ShaderInfo& shader = entry.second;

			glDetachShader(m_ProgramID, shader.ID);
		}
		TRACE_END(3);

		return true;
	}

	bool OpenGLShader::HasUniform(const String& name) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		return location != -1;
	}

	void OpenGLShader::SetUniform1f(const String& name, float value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform1fv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2f(const String& name, const Vector2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform2fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform3f(const String& name, const Vector3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform3fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform4f(const String& name, const Vector4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform4fv(location, 1, (float*)&value);
	}

	void OpenGLShader::SetUniform1i(const String& name, int32 value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform1iv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2i(const String& name, const IVector2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform2iv(location, 1, (int32*)&value);
	}

	void OpenGLShader::SetUniform3i(const String& name, const IVector3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform3iv(location, 1, (int32*)&value);
	}

	void OpenGLShader::SetUniform4i(const String& name, const IVector4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform4iv(location, 1, (int32*)&value);
	}

	void OpenGLShader::SetUniform1ui(const String& name, uint32 value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform1uiv(location, 1, &value);
	}

	void OpenGLShader::SetUniform2ui(const String& name, const UVector2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform2uiv(location, 1, (uint32*)&value);
	}

	void OpenGLShader::SetUniform3ui(const String& name, const UVector3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform3uiv(location, 1, (uint32*)&value);
	}

	void OpenGLShader::SetUniform4ui(const String& name, const UVector4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniform4uiv(location, 1, (uint32*)&value);
	}

	void OpenGLShader::SetUniformMatrix2f(const String& name, const Matrix2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix2x3f(const String& name, const Matrix2x3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix2x3fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix2x4f(const String& name, const Matrix2x4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix2x4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3f(const String& name, const Matrix3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3x2f(const String& name, const Matrix3x2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix3x2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix3x4f(const String& name, const Matrix3x4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix3x4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4f(const String& name, const Matrix4& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4x2f(const String& name, const Matrix4x2& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix4x2fv(location, 1, false, (float*)&value);
	}

	void OpenGLShader::SetUniformMatrix4x3f(const String& name, const Matrix4x3& value) const
	{
		TRACE_FUNCTION();

		glUseProgram(m_ProgramID);
		int32 location = GetUniformLocation(name);
		glUniformMatrix4x3fv(location, 1, false, (float*)&value);
	}

	int32 OpenGLShader::GetUniformLocation(const String& name) const
	{
		TRACE_FUNCTION();

		int32 location;

		auto it = m_UniformCache.find(name);
		if (it == m_UniformCache.end())
		{
			TRACE_BEGIN(0, "OpenGLShader - Uniform cache miss - glGetUniformLocation");
			location = glGetUniformLocation(m_ProgramID, name.c_str());
			if (location != -1) // -1 means the uniform hasn't been found
			{
				m_UniformCache[name] = location;
			}
			else
			{
				LOG_WARN("Cannot find uniform named '{0}'!", name.c_str());
				m_UniformCache[name] = -1;
			}
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
			const ShaderInfo& shader = entry.second;

			glDeleteShader(shader.ID);
		}

		m_Shaders.clear();
	}

	void OpenGLShader::InvalidateUniformCache()
	{
		m_UniformCache.clear();
	}
}
