#include "IonPCH.h"

#include "Material.h"
#include "Shader.h"

namespace Ion
{
	TShared<Material> Material::Create()
	{
		return MakeShareable(new Material);
	}

	Material::~Material()
	{
	}

	void Material::SetShader(const TShared<Shader>& shader)
	{
		m_Shader = shader;
	}

	void Material::CreateParameter(const String& name, EMaterialParameterType type)
	{
		// You cannot add the same parameter twice!
		ionassert(!HasParameter(name));

		switch (type)
		{
		case Ion::EMaterialParameterType::Float:
			m_Parameters.insert({ name, new TMaterialParameter<float> });
			break;
		case Ion::EMaterialParameterType::Float2:
			m_Parameters.insert({ name, new TMaterialParameter<Vector2> });
			break;
		case Ion::EMaterialParameterType::Float3:
			m_Parameters.insert({ name, new TMaterialParameter<Vector3> });
			break;
		case Ion::EMaterialParameterType::Float4:
			m_Parameters.insert({ name, new TMaterialParameter<Vector4> });
			break;
		case Ion::EMaterialParameterType::Bool:
			m_Parameters.insert({ name, new TMaterialParameter<bool> });
			break;
		default:
			LOG_WARN("Unknown parameter type for parameter named '{0}'!", name);
			break;
		}
	}

	void Material::RemoveParameter(const String& name)
	{
		const auto itParams = m_Parameters.find(name);
		if (itParams != m_Parameters.end())
		{
			m_Parameters.erase(itParams);
			// Remove the uniform link too if it exists
			const auto itLinks = m_UniformLinks.find(name);
			if (itLinks != m_UniformLinks.end())
			{
				m_UniformLinks.erase(itLinks);
			}
		}
		else
		{
			LOG_WARN("Parameter '{0}' does not exist!", name);
		}
	}

	void Material::LinkParameterToUniform(const String& name, const String& uniformName)
	{
		if (!HasParameter(name))
		{
			// This is probably due to a user error
			LOG_WARN("Parameter '{0}' does not exist!", name);
			debugbreakd(); 
			return;
		}

		if (!m_Shader->HasUniform(uniformName))
		{
			LOG_WARN("The shader used in the material does not have a uniform named '{0}'!", uniformName);
			return;
		}

		m_UniformLinks[name] = uniformName;
	}

	void Material::UpdateShaderUniforms() const
	{
		TRACE_FUNCTION();

		for (const auto& link : m_UniformLinks)
		{
			const String& parameterName = link.first;
			const String& uniformName = link.second;

			// Uniform link cannot be created if a parameter doesn't exist, so find() is not needed here.
			const auto& paramPtr = m_Parameters.at(parameterName);
			EMaterialParameterType type = ExtractParameterType(paramPtr);
			switch (type)
			{
				case EMaterialParameterType::Float:
				{
					TMaterialParameter<float>* parameter = reinterpret_cast<TMaterialParameter<float>*>(paramPtr);
					m_Shader->SetUniform1f(uniformName, parameter->GetValue());
				}
				break;
				case EMaterialParameterType::Float2:
				{
					TMaterialParameter<Vector2>* parameter = reinterpret_cast<TMaterialParameter<Vector2>*>(paramPtr);
					m_Shader->SetUniform2f(uniformName, parameter->GetValue());
				}
				break;
				case EMaterialParameterType::Float3:
				{
					TMaterialParameter<Vector3>* parameter = reinterpret_cast<TMaterialParameter<Vector3>*>(paramPtr);
					m_Shader->SetUniform3f(uniformName, parameter->GetValue());
				}
				break;
				case EMaterialParameterType::Float4:
				{
					TMaterialParameter<Vector4>* parameter = reinterpret_cast<TMaterialParameter<Vector4>*>(paramPtr);
					m_Shader->SetUniform4f(uniformName, parameter->GetValue());
				}
				break;
				case EMaterialParameterType::Bool:
				{
					TMaterialParameter<bool>* parameter = reinterpret_cast<TMaterialParameter<bool>*>(paramPtr);
					m_Shader->SetUniform1ui(uniformName, parameter->GetValue());
				}
				break;
			}
		}
	}

	Material::Material()
	{
	}
}
