#include "IonPCH.h"

#include "Material.h"
#include "RHI/Texture.h"
#include "RHI/Shader.h"

namespace Ion
{
	TShared<Material> Material::Create()
	{
		return MakeShareable(new Material);
	}

	Material::~Material()
	{
		for (auto& param : m_Parameters)
		{
			void* paramPtr = param.second;

			if (paramPtr)
			{
				auto textureParamPtr = (TMaterialParameter<TShared<RHITexture>>*)paramPtr;
				if (m_TextureParameters.find(textureParamPtr) != m_TextureParameters.end())
					delete textureParamPtr;
				else
					delete paramPtr;
			}
		}
	}

	void Material::SetShader(const TShared<RHIShader>& shader)
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
		case Ion::EMaterialParameterType::Texture2D:
			{
				uint32 slot = (uint32)m_TextureParameters.size();
				ionassert(slot < 16, "There can't be more than 16 textures in one material!");
				TMaterialParameter<TShared<RHITexture>>* param = new TMaterialParameter<TShared<RHITexture>>(slot);
				m_Parameters.insert({ name, param });
				m_TextureParameters.insert(param);

				char samplerUniformName[20];
				sprintf_s(samplerUniformName, "g_Samplers[%u]", param->m_ReservedSlot);
				LinkParameterToUniform(name, samplerUniformName);
			}
			break;
		default:
			LOG_WARN("Unknown parameter type for parameter named '{0}'!", name);
			break;
		}
	}

	void Material::RemoveParameter(const String& name)
	{
#pragma warning(disable:6001)
		const auto itParams = m_Parameters.find(name);
		if (itParams != m_Parameters.end())
		{
			void* paramPtr = itParams->second;

			m_Parameters.erase(itParams);
			// Remove the uniform link too if it exists
			const auto itLinks = m_UniformLinks.find(name);
			if (itLinks != m_UniformLinks.end())
			{
				m_UniformLinks.erase(itLinks);
			}

			if (ExtractParameterType(paramPtr) == EMaterialParameterType::Texture2D)
			{
				m_TextureParameters.erase((TMaterialParameter<TShared<RHITexture>>*)paramPtr);
			}

			// Free the memory
			if (paramPtr)
				delete paramPtr;
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
				case EMaterialParameterType::Texture2D:
				{
					TMaterialParameter<TShared<RHITexture>>* parameter = reinterpret_cast<TMaterialParameter<TShared<RHITexture>>*>(paramPtr);
					m_Shader->SetUniform1i(uniformName, parameter->m_ReservedSlot);
				}
			}
		}
	}

	void Material::BindTextures() const
	{
		for (TMaterialParameter<TShared<RHITexture>>* param : m_TextureParameters)
		{
			if (param->GetValue())
			{
				const TShared<RHITexture> texture = param->GetValue();
				texture->Bind(param->m_ReservedSlot);
			}
			else
			{
				LOG_WARN("Texture in slot {0} is no longer valid!", param->m_ReservedSlot);
			}
		}
	}

	Material::Material()
	{
	}
}
