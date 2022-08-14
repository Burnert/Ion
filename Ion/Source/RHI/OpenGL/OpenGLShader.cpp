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

	void OpenGLShader::AddShaderSource(EShaderType type, const String& source)
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

	Result<void, ShaderCompilationError> OpenGLShader::Compile()
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

			if (!bCompiled)
			{
				OpenGLLogger.Error("Could not compile shader! ({0})", ShaderTypeToString(shader.Type));

				int32 logLength = 0;
				glGetShaderiv(shader.ID, GL_INFO_LOG_LENGTH, &logLength);

				char* message = (char*)_malloca(logLength);
				glGetShaderInfoLog(shader.ID, logLength, &logLength, message);

				OpenGLLogger.Trace("Shader compilation log: \n{0}", message);

				CleanupDeleteShaders();
				
				ionthrow(ShaderCompilationError, "{0} could not be compiled!\n{1}", ShaderTypeToString(shader.Type), message);
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

		if (!bLinked)
		{
			OpenGLLogger.Error("Could not link shader program!");
			glDeleteProgram(m_ProgramID);
			m_ProgramID = 0;

			ionthrow(ShaderCompilationError, "Could not link shader program!");
		}

		TRACE_BEGIN(3, "OpenGLShader - Shader Detachment");
		// Shaders need to be detached after a successful link
		for (auto& entry : m_Shaders)
		{
			const ShaderInfo& shader = entry.second;

			glDetachShader(m_ProgramID, shader.ID);
		}
		TRACE_END(3);

		return Void();
	}

	bool OpenGLShader::IsCompiled()
	{
		return m_bCompiled;
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
				OpenGLLogger.Warn("Cannot find uniform named '{0}'!", name.c_str());
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
