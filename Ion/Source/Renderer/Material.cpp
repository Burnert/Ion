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

	void Material::CreateMaterialProperty(const String& name, EMaterialPropertyType type)
	{
		// You cannot add the same property twice!
		ionassert(!HasMaterialProperty(name));

		switch (type)
		{
		case Ion::EMaterialPropertyType::Float:
			m_Properties.insert({ name, new MaterialProperty<float> });
			break;
		case Ion::EMaterialPropertyType::Float2:
			m_Properties.insert({ name, new MaterialProperty<FVector2> });
			break;
		case Ion::EMaterialPropertyType::Float3:
			m_Properties.insert({ name, new MaterialProperty<FVector3> });
			break;
		case Ion::EMaterialPropertyType::Float4:
			m_Properties.insert({ name, new MaterialProperty<FVector4> });
			break;
		case Ion::EMaterialPropertyType::Bool:
			m_Properties.insert({ name, new MaterialProperty<bool> });
			break;
		default:
			LOG_WARN("Unknown property type for material property '{0}'!", name);
			break;
		}
	}

	void Material::RemoveMaterialProperty(const String& name)
	{
		const auto itProps = m_Properties.find(name);
		if (itProps != m_Properties.end())
		{
			m_Properties.erase(itProps);
			// Remove the uniform link too if it exists
			const auto itLinks = m_UniformLinks.find(name);
			if (itLinks != m_UniformLinks.end())
			{
				m_UniformLinks.erase(itLinks);
			}
		}
		else
		{
			LOG_WARN("Property '{0}' does not exist!", name);
		}
	}

	void Material::LinkPropertyToUniform(const String& name, const String& uniformName)
	{
		if (!HasMaterialProperty(name))
		{
			// This is probably due to a user error
			LOG_WARN("Property '{0}' does not exist!", name);
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

	void Material::UpdateShaderUniforms()
	{
		for (const auto& link : m_UniformLinks)
		{
			const String& propertyName = link.first;
			const String& uniformName = link.second;

			// Uniform link cannot be created if a property doesn't exist, so find() is not needed here.
			const auto& propPtr = m_Properties.at(propertyName);
			EMaterialPropertyType type = ExtractPropertyType(propPtr);
			switch (type)
			{
				case EMaterialPropertyType::Float:
				{
					MaterialProperty<float>* property = reinterpret_cast<MaterialProperty<float>*>(propPtr);
					m_Shader->SetUniform1f(uniformName, property->GetValue());
				}
				break;
				case EMaterialPropertyType::Float2:
				{
					MaterialProperty<FVector2>* property = reinterpret_cast<MaterialProperty<FVector2>*>(propPtr);
					m_Shader->SetUniform2f(uniformName, property->GetValue());
				}
				break;
				case EMaterialPropertyType::Float3:
				{
					MaterialProperty<FVector3>* property = reinterpret_cast<MaterialProperty<FVector3>*>(propPtr);
					m_Shader->SetUniform3f(uniformName, property->GetValue());
				}
				break;
				case EMaterialPropertyType::Float4:
				{
					MaterialProperty<FVector4>* property = reinterpret_cast<MaterialProperty<FVector4>*>(propPtr);
					m_Shader->SetUniform4f(uniformName, property->GetValue());
				}
				break;
				case EMaterialPropertyType::Bool:
				{
					MaterialProperty<bool>* property = reinterpret_cast<MaterialProperty<bool>*>(propPtr);
					m_Shader->SetUniform1ui(uniformName, property->GetValue());
				}
				break;
			}
		}
	}

	Material::Material()
	{
	}
}
