#include "IonPCH.h"

#include "Material.h"
#include "RHI/Shader.h"
#include "RHI/UniformBuffer.h"

#include "Core/Asset/AssetRegistry.h"

#include "Application/EnginePath.h"

namespace Ion
{
	ShaderPermutation::ShaderPermutation(EShaderUsage usage) :
		Shader(RHIShader::Create()),
		Usage(usage),
		bCompiled(false)
	{
	}

	MaterialParameterScalar::MaterialParameterScalar(const String& name, float defaultValue, float min, float max) :
		m_Name(name),
		m_DefaultValue(defaultValue),
		m_MinValue(min),
		m_MaxValue(max)
	{
	}

	EMaterialParameterType MaterialParameterScalar::GetType() const
	{
		return EMaterialParameterType::Scalar;
	}

	const String& MaterialParameterScalar::GetName() const
	{
		return m_Name;
	}

	MaterialParameterVector::MaterialParameterVector(const String& name, Vector4 defaultValue, Vector4 min, Vector4 max) :
		m_Name(name),
		m_DefaultValue(defaultValue),
		m_MinValue(min),
		m_MaxValue(max)
	{
	}

	EMaterialParameterType MaterialParameterVector::GetType() const
	{
		return EMaterialParameterType::Vector;
	}

	const String& MaterialParameterVector::GetName() const
	{
		return m_Name;
	}

	MaterialParameterTexture2D::MaterialParameterTexture2D(const String& name, uint32 slot, Asset defaultValue) :
		m_Name(name),
		m_DefaultValue(defaultValue),
		m_TextureSlot(slot)
	{
	}

	EMaterialParameterType MaterialParameterTexture2D::GetType() const
	{
		return EMaterialParameterType::Texture2D;
	}

	const String& MaterialParameterTexture2D::GetName() const
	{
		return m_Name;
	}

	// MaterialParameterInstance impl ------------------------------------------------------------------------------------

	MaterialParameterInstanceScalar::MaterialParameterInstanceScalar(MaterialParameterScalar* parentParameter, float value) :
		m_Parameter(parentParameter),
		m_Value(value)
	{
		ionassert(m_Parameter);
	}

	IMaterialParameter* MaterialParameterInstanceScalar::GetParameter() const
	{
		return m_Parameter;
	}

	MaterialParameterScalar* MaterialParameterInstanceScalar::GetParameterScalar() const
	{
		return m_Parameter;
	}

	MaterialParameterInstanceVector::MaterialParameterInstanceVector(MaterialParameterVector* parentParameter, const Vector4& value) :
		m_Parameter(parentParameter),
		m_Value(value)
	{
		ionassert(m_Parameter);
	}

	IMaterialParameter* MaterialParameterInstanceVector::GetParameter() const
	{
		return m_Parameter;
	}

	MaterialParameterVector* MaterialParameterInstanceVector::GetParameterVector() const
	{
		return m_Parameter;
	}

	MaterialParameterInstanceTexture2D::MaterialParameterInstanceTexture2D(MaterialParameterTexture2D* parentParameter, Asset value) :
		m_Parameter(parentParameter),
		m_Value(value)
	{
		ionassert(m_Parameter);
	}

	IMaterialParameter* MaterialParameterInstanceTexture2D::GetParameter() const
	{
		return m_Parameter;
	}

	MaterialParameterTexture2D* MaterialParameterInstanceTexture2D::GetParameterTexture2D() const
	{
		return m_Parameter;
	}

	// End of MaterialParameterInstance impl ------------------------------------------------------------------------------------

	TShared<Material> Material::Create()
	{
		return MakeShareable(new Material);
	}

	TShared<Material> Material::CreateFromAsset(Asset materialAsset)
	{
		return MakeShareable(new Material(materialAsset));
	}

	void Material::CompileShaders()
	{
		for (auto& [usage, shader] : m_Shaders)
		{
			EShaderUsage shaderUsage = usage;
			ShaderPermutation& shaderPerm = shader;

			ionassert(shaderPerm.Shader);
			ionassert(!shaderPerm.bCompiled);

			shaderPerm.Shader->AddShaderSource(EShaderType::Vertex, m_MaterialCode);
			shaderPerm.Shader->AddShaderSource(EShaderType::Pixel, m_MaterialCode);

			// Compile the shaders using the Engine Task Queue
			AsyncTask compileTask([material = shared_from_this(), shaderUsage, &shaderPerm](IMessageQueueProvider& queue)
			{
				if (shaderPerm.Shader->Compile())
				{
					queue.PushMessage(FTaskMessage([material, shaderUsage, &shaderPerm]
					{
						ionassert(material->m_Shaders.find(shaderUsage) != material->m_Shaders.end(),
							"Shader usage had been removed before the shader was compiled.");
						shaderPerm.bCompiled = true;
					}));
				}
			});
			compileTask.Schedule();
		}
	}

	void Material::BindShaders() const
	{
		ionassert(std::all_of(m_Shaders.begin(), m_Shaders.end(), [](auto& entry)
		{
			return entry.second.bCompiled;
		}), "Not every shader has been compiled.");
	}

	void Material::AddUsage(EShaderUsage usage)
	{
		m_Usage |= (uint64)usage;
		m_Shaders.emplace(usage, ShaderPermutation(usage));
	}

	void Material::RemoveUsage(EShaderUsage usage)
	{
		m_Usage |= (uint64)usage;
		m_Shaders.erase(usage);
	}

	bool Material::IsUsableWith(EShaderUsage usage)
	{
		return GetBitflag(m_Usage, (decltype(m_Usage))usage);
	}

	void Material::SetMaterialCode(const String& code)
	{
		m_MaterialCode = code;
		Invalidate();
	}

	void Material::Invalidate()
	{
		m_bInvalid = true;
	}

	bool Material::CompileMaterialCode()
	{
		if (m_MaterialCode.empty())
		{
			LOG_ERROR("Cannot compile blank material code.");
			return false;
		}

		for (auto& [usage, shader] : m_Shaders)
		{
			if (!CompileMaterialCode_Internal(usage, shader))
				return false;
		}

		return true;
	}

	bool Material::CompileMaterialCode(EShaderUsage usage)
	{
		if (m_MaterialCode.empty())
		{
			LOG_ERROR("Cannot compile blank material code.");
			return false;
		}

		if (m_Shaders.find(usage) == m_Shaders.end())
		{
			LOG_ERROR("Material cannot be compiled for the specified usage ({0}).", ToString(usage));
			return false;
		}

		return CompileMaterialCode_Internal(usage, m_Shaders.at(usage));
	}

	Material::Material() :
		m_Usage(0),
		m_bInvalid(false),
		m_UsedTextureSlotsMask(0)
	{
	}

	Material::Material(Asset materialAsset) :
		m_Usage(0),
		m_bInvalid(false),
		m_UsedTextureSlotsMask(0)
	{
		ParseAsset(materialAsset);
		BuildConstantBuffer();
	}

	bool Material::ParseAsset(Asset materialAsset)
	{
		ionassert(materialAsset);

		const FilePath& path = materialAsset->GetDefinitionPath();

		String assetDefinition;
		File::ReadToString(path, assetDefinition);

		TShared<XMLDocument> xml = MakeShared<XMLDocument>(assetDefinition);

		// <IonAsset>
		XMLNode* nodeIonAsset = xml->XML().first_node(IASSET_NODE_IonAsset);
		IASSET_CHECK_NODE(nodeIonAsset, IASSET_NODE_IonAsset, path);

		// <Material>
		XMLNode* nodeMaterial = nodeIonAsset->first_node(IASSET_NODE_Material);
		IASSET_CHECK_NODE(nodeMaterial, IASSET_NODE_Material, path);

		// <Code>
		XMLNode* nodeMaterialCode = nodeMaterial->first_node(IASSET_NODE_Material_Code);
		IASSET_CHECK_NODE(nodeMaterialCode, IASSET_NODE_Material_Code, path);

		XMLAttribute* attrMaterialCodeSource = nodeMaterialCode->first_attribute(IASSET_ATTR_source);
		if (attrMaterialCodeSource)
		{
			char* csMaterialCodeSource = attrMaterialCodeSource->value();
			ionassert(csMaterialCodeSource);
			if (strlen(csMaterialCodeSource) == 0)
			{
				LOG_ERROR("Material source code path not specified.");
				return false;
			}

			if (!LoadExternalMaterialCode(EnginePath::GetShadersPath() + L"Material" + StringConverter::StringToWString(csMaterialCodeSource)))
				return false;
		}
		else
		{
			char* csMaterialCode = nodeMaterialCode->value();
			ionassert(csMaterialCode);

			if (!ParseMaterialCode(csMaterialCode))
				return false;
		}

		// <Parameter> nodes
		XMLNode* nodeMaterialParameter = nodeMaterial->first_node(IASSET_NODE_Material_Parameter);
		while (nodeMaterialParameter)
		{
			if (!ParseMaterialParameter(nodeMaterialParameter, path))
				return false;

			// Get next <Parameter>
			nodeMaterialParameter = nodeMaterialParameter->next_sibling(IASSET_NODE_Material_Parameter);
		}

		return true;
	}

	bool Material::ParseMaterialParameter(XMLNode* parameterNode, const FilePath& path)
	{
		ionassert(parameterNode);

		// type=
		XMLAttribute* attrMaterialParamType = parameterNode->first_attribute(IASSET_ATTR_type);
		IASSET_CHECK_ATTR(attrMaterialParamType, IASSET_ATTR_type, IASSET_NODE_Material_Parameter, path);

		char* csParamType = attrMaterialParamType->value();
		ionassert(csParamType);
		if (strlen(csParamType) == 0)
			return false;

		EMaterialParameterType paramType = MaterialParameterTypeFromString(csParamType);

		// name=
		XMLAttribute* attrMaterialParamName = parameterNode->first_attribute(IASSET_ATTR_name);
		IASSET_CHECK_ATTR(attrMaterialParamName, IASSET_ATTR_name, IASSET_NODE_Material_Parameter, path);

		char* csParamName = attrMaterialParamName->value();
		ionassert(csParamName);
		if (strlen(csParamName) == 0)
			return false;

		IMaterialParameter* parameter = AddParameter(csParamName, paramType);
		if (!parameter)
		{
			LOG_ERROR("Cannot add a material parameter.");
			return false;
		}

		// <Default>
		XMLNode* nodeDefault = parameterNode->first_node(IASSET_NODE_Material_Parameter_Default);
		if (nodeDefault)
		{
			char* csDefault = nodeDefault->value();
			// @TODO: Parse the default accordingly
			switch (paramType)
			{
				case EMaterialParameterType::Scalar:
				{
					MaterialParameterScalar* scalarParam = (MaterialParameterScalar*)parameter;

					char* pEnd;
					float value = strtof(csDefault, &pEnd);
					if (pEnd == csDefault || errno == ERANGE)
					{
						LOG_ERROR("Invalid default value.");
						return false;
					}
					scalarParam->SetDefaultValue(value);
					break;
				}
				case EMaterialParameterType::Vector:
				{
					MaterialParameterVector* vectorParam = (MaterialParameterVector*)parameter;

					Vector4 value;
					float* currentValue = (float*)&value;
					char* pEnd;
					// Is the currentValue still inside the Vector4
					while (currentValue - (float*)&value < 4)
					{
						*currentValue++ = strtof(csDefault, &pEnd);
						if (pEnd == csDefault || errno == ERANGE)
						{
							LOG_ERROR("Invalid default value.");
							return false;
						}
						// Omit the space between components;
						if (*pEnd)
							csDefault = pEnd + 1;
					}
					vectorParam->SetDefaultValue(value);
					break;
				}
				case EMaterialParameterType::Texture2D:
				{
					MaterialParameterTexture2D* texture2dParam = (MaterialParameterTexture2D*)parameter;

					GUID assetGuid(csDefault);
					if (!assetGuid)
						return false;

					AssetDefinition* assetDef = AssetRegistry::Find(assetGuid);
					if (!assetDef)
					{
						LOG_WARN("Asset {0} has not been found.", assetGuid.ToString());
						return false;
					}
					texture2dParam->SetDefaultValue(assetDef->GetHandle());

					break;
				}
			}
		}

		return true;
	}

	// Code parsing ==============================================================

	enum class ECodeTokenType
	{
		None,
		Text,
		Number,
		Annotation,
		Operator,
	};

	struct CodeToken
	{
		String Text;
		ECodeTokenType Type;
	};

	static bool IsTextKindOfToken(ECodeTokenType type)
	{
		return IsAnyOf(type,
			ECodeTokenType::Text,
			ECodeTokenType::Annotation);
	}

	static bool IsLetter(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	static bool IsNumber(char c)
	{
		return (c >= '0' && c <= '9');
	}

	enum class EParseState
	{
		None,
		NewToken,
		ExistingToken
	};

	bool Material::ParseMaterialCode(const String& code)
	{
		// @TODO: Finish this whole thing but do it correctly and not like spaghetti.

		if (code.empty())
		{
			LOG_ERROR("Material Code is empty!");
			return false;
		}

		// 1. Briefly tokenize the code to get the parameters

		TArray<CodeToken> tokens;
		ECodeTokenType currentTokenType = ECodeTokenType::None;
		String* currentTokenText = nullptr;
		EParseState parseState = EParseState::None;

		const char* currentChar = code.data();
		while (currentChar)
		{
			// Evaluate if a new token should be created

			if (parseState == EParseState::ExistingToken || parseState == EParseState::None)
			{
				// Annotation
				if (*currentChar == '@')
				{
					parseState = EParseState::NewToken;
					currentTokenType = ECodeTokenType::Annotation;

					tokens.emplace_back(CodeToken { "@", currentTokenType });
					currentTokenText = &tokens.back().Text;
					currentTokenText->reserve(16);

					parseState = EParseState::ExistingToken;
					// Go to the next character immediately
					if (!++currentChar)
						break;
					continue;
				}
				// Text token
				else if (!IsTextKindOfToken(currentTokenType) && IsLetter(*currentChar))
				{
					parseState = EParseState::NewToken;
					currentTokenType = ECodeTokenType::Text;

					tokens.emplace_back(CodeToken { "", currentTokenType });
					currentTokenText = &tokens.back().Text;
					currentTokenText->reserve(16);
				}
			}

			// 

			if (IsTextKindOfToken(currentTokenType))
			{
				parseState = EParseState::ExistingToken;
				// Add the character if it follows variable naming pattern
				if (IsLetter(*currentChar) || (!currentTokenText->empty() && IsNumber(*currentChar)))
				{
					*currentTokenText += *currentChar;
				}
			}
			else if (currentTokenType == ECodeTokenType::Number)
			{
				parseState = EParseState::ExistingToken;

				if (IsNumber(*currentChar))
				{
					*currentTokenText += *currentChar;
;				}
			}

			++currentChar;
		}

		return true;
	}

	// End of Code parsing ==============================================================

	bool Material::LoadExternalMaterialCode(const FilePath& path)
	{
		String source;
		if (!File::ReadToString(path, source))
			return false;

		SetMaterialCode(source);

		return true;
	}

	IMaterialParameter* Material::AddParameter(const String& name, EMaterialParameterType type)
	{
		auto it = m_Parameters.find(name);
		if (it != m_Parameters.end())
		{
			LOG_ERROR("Cannot add a material parameter, because a parameter with name \"{0}\" already exists.", name);
			return nullptr;
		}

		IMaterialParameter* parameter = nullptr;

		switch (type)
		{
		case EMaterialParameterType::Scalar:
			parameter = new MaterialParameterScalar(name);
			break;
		case EMaterialParameterType::Vector:
			parameter = new MaterialParameterVector(name);
			break;
		case EMaterialParameterType::Texture2D:
			if (GetTextureParameterCount() == 16)
			{
				LOG_ERROR("A Material cannot have more than 16 Texture Parameters.");
				return nullptr;
			}
			uint32 freeSlot = GetFirstFreeTextureSlot();
			parameter = new MaterialParameterTexture2D(name, freeSlot);

			break;
		}

		if (!parameter)
			return nullptr;

		m_Parameters.emplace(name, parameter);

		return parameter;
	}

	bool Material::RemoveParameter(const String& name)
	{
		auto it = m_Parameters.find(name);
		if (it == m_Parameters.end())
		{
			LOG_ERROR("Cannot find a parameter with name \"{0}\".", name);
			return false;
		}

		ionassert(it->second, "MaterialParameter \"%s\" is null.", name.c_str());

		if (it->second->GetType() == EMaterialParameterType::Texture2D)
		{
			MaterialParameterTexture2D* textureParam = (MaterialParameterTexture2D*)it->second;
			uint32 slot = textureParam->GetSlot();
			
			UnsetBitflags(m_UsedTextureSlotsMask, Bitflag(slot));
		}
		
		delete it->second;
		m_Parameters.erase(it);

		return true;
	}

	void Material::BuildConstantBuffer()
	{
		UniformBufferFactory factory;

		// 1. Sort the parameters

		TArray<MaterialParameterVector*> vectorParams;
		TArray<MaterialParameterScalar*> scalarParams;
		vectorParams.reserve(m_Parameters.size());
		scalarParams.reserve(m_Parameters.size());

		for (auto& [name, parameter] : m_Parameters)
		{
			switch (parameter->GetType())
			{
				case EMaterialParameterType::Scalar:
				{
					scalarParams.emplace_back((MaterialParameterScalar*)parameter);
					break;
				}
				case EMaterialParameterType::Vector:
				{
					vectorParams.emplace_back((MaterialParameterVector*)parameter);
					break;
				}
			}
		}

		// 2. Add the parameters in order

		for (MaterialParameterVector* vectorParam : vectorParams)
		{
			factory.Add(vectorParam->GetName(), EUniformType::Float4);
		}

		for (MaterialParameterScalar* scalarParam : scalarParams)
		{
			factory.Add(scalarParam->GetName(), EUniformType::Float);
		}

		m_MaterialConstants = factory.Construct();
	}

	uint32 Material::GetTextureParameterCount() const
	{
		return (uint32)std::bitset<32>(m_UsedTextureSlotsMask).count();
	}

	uint32 Material::GetFirstFreeTextureSlot() const
	{
		unsigned long index = 0;
		// @TODO: Don't use this function here, make some kind of platform abstraction
		_BitScanForward(&index, ~m_UsedTextureSlotsMask);
		return index;
	}

	bool Material::CompileMaterialCode_Internal(EShaderUsage usage, ShaderPermutation& outShader)
	{
		ionassert(!m_MaterialCode.empty());
		ionassert(m_Shaders.find(usage) != m_Shaders.end());
		ionassert(outShader.Shader);
		ionassert(outShader.bCompiled == false);
		ionassert(outShader.Usage == usage);

		String vertexSource;
		String pixelSource;

		outShader.Shader->AddShaderSource(EShaderType::Vertex, vertexSource);
		outShader.Shader->AddShaderSource(EShaderType::Pixel, pixelSource);

		return true;
	}

	void MaterialInstance::TransferParameters() const
	{
		// Update the constant buffer fields from parameters
		// Textures need to be loaded and bound normally

		TShared<RHIUniformBufferDynamic> constants = m_ParentMaterial->m_MaterialConstants;
		constants->Bind(2);

		for (auto& [name, parameter] : m_ParameterInstances)
		{
			switch (parameter->GetType())
			{
				case EMaterialParameterType::Scalar:
				{
					MaterialParameterInstanceScalar* scalarParamInstance = (MaterialParameterInstanceScalar*)parameter;
					constants->SetUniformValue(name, scalarParamInstance->GetValue());
					break;
				}
				case EMaterialParameterType::Vector:
				{
					MaterialParameterInstanceVector* vectorParamInstance = (MaterialParameterInstanceVector*)parameter;
					constants->SetUniformValue(name, vectorParamInstance->GetValue());
					break;
				}
				//case EMaterialParameterType::Texture2D:
				//{
				//	MaterialParameterInstanceTexture2D* textureParamInstance = (MaterialParameterInstanceTexture2D*)parameter;
				//	MaterialParameterTexture2D* textureParam = textureParamInstance->GetParameterTexture2D();
				//	break;
				//}
			}
		}
	}

	MaterialInstance::MaterialInstance(const TShared<Material>& parentMaterial) :
		m_ParentMaterial(parentMaterial)
	{
		ionassert(parentMaterial);

		CreateParameterInstances();
	}

	void MaterialInstance::CreateParameterInstances()
	{
		for (auto& [name, parameter] : m_ParentMaterial->m_Parameters)
		{
			IMaterialParameterInstance* instance = nullptr;

			switch (parameter->GetType())
			{
				case EMaterialParameterType::Scalar:
				{
					MaterialParameterScalar* scalarParam = (MaterialParameterScalar*)parameter;
					instance = new MaterialParameterInstanceScalar(scalarParam, scalarParam->GetDefaultValue());
					break;
				}
				case EMaterialParameterType::Vector:
				{
					MaterialParameterVector* vectorParam = (MaterialParameterVector*)parameter;
					instance = new MaterialParameterInstanceVector(vectorParam, vectorParam->GetDefaultValue());
					break;
				}
				case EMaterialParameterType::Texture2D:
				{
					MaterialParameterTexture2D* textureParam = (MaterialParameterTexture2D*)parameter;
					instance = new MaterialParameterInstanceTexture2D(textureParam, textureParam->GetDefaultValue());
					break;
				}
			}

			m_ParameterInstances.emplace(name, instance);
		}
	}

	TShared<MaterialInstance> MaterialInstance::Create(const TShared<Material>& parentMaterial)
	{
		return MakeShareable(new MaterialInstance(parentMaterial));
	}

	IMaterialParameterInstance* MaterialInstance::GetMaterialParameterInstance(const String& name) const
	{
		ionassert(m_ParentMaterial);
		ionassert(m_ParentMaterial->m_Parameters.find(name) != m_ParentMaterial->m_Parameters.end());

		auto it = m_ParameterInstances.find(name);
		if (it != m_ParameterInstances.end())
		{
			return it->second;
		}
		return nullptr;
	}
}
