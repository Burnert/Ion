#include "IonPCH.h"

#include "Material.h"
#include "RHI/Shader.h"
#include "RHI/UniformBuffer.h"

#include "Core/Asset/AssetRegistry.h"
#include "Core/Asset/AssetParser.h"

#include "Application/EnginePath.h"

namespace Ion
{
	ShaderPermutation::ShaderPermutation(EShaderUsage usage) :
		Shader(RHIShader::Create()),
		Usage(usage),
		bCompiled(false)
	{
	}

	void IMaterialParameter::SetValues(const TMaterialParameterTypeVariant& def, const TMaterialParameterTypeVariant& min, const TMaterialParameterTypeVariant& max)
	{
		// @TODO: There should be a better way to set these values
		
		EMaterialParameterType type = GetType();
		switch (type)
		{
			case EMaterialParameterType::Scalar:
			{
				ionassert(std::holds_alternative<float>(def));
				ionassert(std::holds_alternative<float>(min));
				ionassert(std::holds_alternative<float>(max));

				MaterialParameterScalar* param = (MaterialParameterScalar*)this;
				param->SetDefaultValue(std::get<float>(def));
				param->SetMinValue(std::get<float>(min));
				param->SetMaxValue(std::get<float>(max));
				break;
			}
			case EMaterialParameterType::Vector:
			{
				ionassert(std::holds_alternative<Vector4>(def));
				ionassert(std::holds_alternative<Vector4>(min));
				ionassert(std::holds_alternative<Vector4>(max));

				MaterialParameterVector* param = (MaterialParameterVector*)this;
				param->SetDefaultValue(std::get<Vector4>(def));
				param->SetMinValue(std::get<Vector4>(min));
				param->SetMaxValue(std::get<Vector4>(max));
				break;
			}
			case EMaterialParameterType::Texture2D:
			{
				ionassert(std::holds_alternative<GUID>(def));

				MaterialParameterTexture2D* param = (MaterialParameterTexture2D*)this;
				param->SetDefaultValue(Asset::Find(std::get<GUID>(def)));
				break;
			}
		}
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

	void IMaterialParameterInstance::SetValue(const TMaterialParameterTypeVariant& value)
	{
		// @TODO: There should be a better way to set these values

		EMaterialParameterType type = GetType();
		switch (type)
		{
			case EMaterialParameterType::Scalar:
			{
				ionassert(std::holds_alternative<float>(value));

				MaterialParameterInstanceScalar* param = (MaterialParameterInstanceScalar*)this;
				param->SetValue(std::get<float>(value));
				break;
			}
			case EMaterialParameterType::Vector:
			{
				ionassert(std::holds_alternative<Vector4>(value));

				MaterialParameterInstanceVector* param = (MaterialParameterInstanceVector*)this;
				param->SetValue(std::get<Vector4>(value));
				break;
			}
			case EMaterialParameterType::Texture2D:
			{
				ionassert(std::holds_alternative<GUID>(value));

				MaterialParameterInstanceTexture2D* param = (MaterialParameterInstanceTexture2D*)this;
				param->SetValue(Asset::Find(std::get<GUID>(value)));
				break;
			}
		}
	}

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
		m_Parameter(parentParameter)
	{
		ionassert(m_Parameter);
		SetValue(value);
	}

	IMaterialParameter* MaterialParameterInstanceTexture2D::GetParameter() const
	{
		return m_Parameter;
	}

	MaterialParameterTexture2D* MaterialParameterInstanceTexture2D::GetParameterTexture2D() const
	{
		return m_Parameter;
	}

	void MaterialParameterInstanceTexture2D::Bind(uint32 slot)
	{
		if (m_Texture)
		{
			m_Texture->Bind(slot);
		}
	}

	void MaterialParameterInstanceTexture2D::UpdateTexture()
	{
		if (!m_Value)
		{
			m_TextureResource = nullptr;
			return;
		}

		// The asset handle has changed
		if (!m_TextureResource || m_Value != m_TextureResource->GetAssetHandle())
		{
			m_TextureResource = TextureResource::Query(m_Value);

			// @TODO: Make sure the instance is not deleted before this gets done
			m_TextureResource->Take([this](const TextureResourceRenderDataShared& data)
			{
				m_Texture = data.Texture;
			});
		}
	}

	// End of MaterialParameterInstance impl ------------------------------------------------------------------------------------

	MaterialRegistry* MaterialRegistry::s_Instance = nullptr;

	TShared<Material> MaterialRegistry::QueryMaterial(Asset materialAsset)
	{
		MaterialRegistry& instance = Get();

		TShared<Material> material;

		auto it = instance.m_Materials.find(materialAsset);
		if (it == instance.m_Materials.end() || it->second.expired())
		{
			material = Material::CreateFromAsset(materialAsset);
			instance.m_Materials.emplace(materialAsset, material);
		}
		else
		{
			material = it->second.lock();
		}

		return material;
	}

	TShared<MaterialInstance> MaterialRegistry::QueryMaterialInstance(Asset materialInstanceAsset)
	{
		MaterialRegistry& instance = Get();

		TShared<MaterialInstance> materialInstance;

		auto it = instance.m_MaterialInstances.find(materialInstanceAsset);
		if (it == instance.m_MaterialInstances.end() || it->second.expired())
		{
			materialInstance = MaterialInstance::CreateFromAsset(materialInstanceAsset);
			instance.m_MaterialInstances.emplace(materialInstanceAsset, materialInstance);
		}
		else
		{
			materialInstance = it->second.lock();
		}

		return materialInstance;
	}

	// Material --------------------------------------------------------------------------------------

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

	void Material::CompileShaders(const FOnShadersCompiled& onCompiled)
	{
		TShared<ShadersCompiledCounter> counter = MakeShared<ShadersCompiledCounter>((uint32)m_Shaders.size());

		for (auto& [usage, shader] : m_Shaders)
		{
			EShaderUsage shaderUsage = usage;
			ShaderPermutation& shaderPerm = shader;

			ionassert(shaderPerm.Shader);
			ionassert(!shaderPerm.bCompiled);

			shaderPerm.Shader->AddShaderSource(EShaderType::Vertex, m_MaterialCode);
			shaderPerm.Shader->AddShaderSource(EShaderType::Pixel, m_MaterialCode);

			// Compile the shaders using the Engine Task Queue
			AsyncTask compileTask([material = shared_from_this(), shaderUsage, &shaderPerm, counter, onCompiled](IMessageQueueProvider& queue)
			{
				if (shaderPerm.Shader->Compile())
				{
					queue.PushMessage(FTaskMessage([material, shaderUsage, &shaderPerm, counter, onCompiled]
					{
						ionassert(material->m_Shaders.find(shaderUsage) != material->m_Shaders.end(),
							"Shader usage had been removed before the shader was compiled.");
						shaderPerm.bCompiled = true;

						counter->Inc();
						if (counter->Done())
						{
							onCompiled(shaderPerm);
						}
					}));
				}
			});
			compileTask.Schedule();
		}
	}

	bool Material::BindShader(EShaderUsage usage) const
	{
		ionassert(m_Shaders.find(usage) != m_Shaders.end());

		if (!m_Shaders.at(usage).bCompiled)
			return false;
		
		m_Shaders.at(usage).Shader->Bind();

		return true;
	}

	const TShared<RHIShader>& Material::GetShader(EShaderUsage usage) const
	{
		ionassert(m_Shaders.find(usage) != m_Shaders.end());

		return m_Shaders.at(usage).Shader;
	}

	void Material::UpdateConstantBuffer() const
	{
		m_MaterialConstants->UpdateData();
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
		return m_Usage & (uint64)usage;
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

	Material::~Material()
	{
		DestroyParameters();
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
		m_UsedTextureSlotsMask(0),
		m_Asset(materialAsset)
	{
		ParseAsset(materialAsset);
		BuildConstantBuffer();
	}

	bool Material::ParseAsset(Asset materialAsset)
	{
		ionassert(materialAsset);
		ionassert(materialAsset->GetType() == EAssetType::Material);

		XMLParserResult result = MaterialAssetParser(materialAsset)
			.BeginAsset() // <IonAsset>
			.BeginMaterial() // <Material>

			.LoadCode([this](String source) // <Code>
			{
				FilePath path = EnginePath::GetShadersPath() + L"Material" + StringConverter::StringToWString(source);
				return LoadExternalMaterialCode(path);
			})

			.ParseParameters([this](
				String name,
				EMaterialParameterType type,
				const MaterialAssetParser::ParameterValues& values)
			{
				IMaterialParameter* parameter = AddParameter(name, type);
				parameter->SetValues(values.Default, values.Min, values.Max);
			})

			.EndMaterial() // </Material>
			.Finalize(); // </IonAsset>

		return result.OK();
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

	void Material::DestroyParameters()
	{
		if (m_Parameters.empty())
			return;

		for (auto& [name, param] : m_Parameters)
		{
			delete param;
		}
		m_Parameters.clear();
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

	// Material Instance ------------------------------------------------------------------------------

	TShared<MaterialInstance> MaterialInstance::Create(const TShared<Material>& parentMaterial)
	{
		return MakeShareable(new MaterialInstance(parentMaterial));
	}

	TShared<MaterialInstance> MaterialInstance::CreateFromAsset(Asset materialInstanceAsset)
	{
		return MakeShareable(new MaterialInstance(materialInstanceAsset));
	}

	void MaterialInstance::BindTextures() const
	{
		for (MaterialParameterInstanceTexture2D* textureParam : m_TextureParameterInstances)
		{
			textureParam->Bind(textureParam->GetParameterTexture2D()->GetSlot());
		}
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
			}
		}
	}

	MaterialInstance::~MaterialInstance()
	{
		DestroyParameterInstances();
	}

	MaterialInstance::MaterialInstance(const TShared<Material>& parentMaterial) :
		m_ParentMaterial(parentMaterial)
	{
		ionassert(parentMaterial);

		CreateParameterInstances();
	}

	MaterialInstance::MaterialInstance(Asset materialInstanceAsset) :
		m_Asset(materialInstanceAsset)
	{
		ionassert(materialInstanceAsset);

		if (!ParseAsset(materialInstanceAsset))
		{
			ionassert(false);
			return;
		}
	}

	void MaterialInstance::SetParentMaterial(const TShared<Material>& material)
	{
		DestroyParameterInstances();
		m_ParentMaterial = material;
		CreateParameterInstances();
	}

	void MaterialInstance::CreateParameterInstances()
	{
		ionassert(m_ParameterInstances.empty(), "Destroy existing instances before creating new ones.");

		ionexcept(m_ParentMaterial, "Cannot create parameter instances. Parent Material is not set.")
			return;

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
					m_TextureParameterInstances.emplace((MaterialParameterInstanceTexture2D*)instance);
					break;
				}
			}

			m_ParameterInstances.emplace(name, instance);
		}
	}

	void MaterialInstance::DestroyParameterInstances()
	{
		if (m_ParameterInstances.empty())
			return;

		for (auto& [name, param] : m_ParameterInstances)
		{
			delete param;
		}
		m_ParameterInstances.clear();
		m_TextureParameterInstances.clear();
	}

	bool MaterialInstance::ParseAsset(Asset materialInstanceAsset)
	{
		ionassert(materialInstanceAsset);
		ionassert(materialInstanceAsset->GetType() == EAssetType::MaterialInstance);

		XMLParserResult result = MaterialInstanceAssetParser(materialInstanceAsset)
			.BeginAsset() // <IonAsset>
			.BeginMaterialInstance([this](const Asset& asset) // <MaterialInstance>
			{
				SetParentMaterial(MaterialRegistry::QueryMaterial(asset));
			})

			.ParseParameterInstances([this](
				String name,
				EMaterialParameterType type,
				const MaterialInstanceAssetParser::ParameterInstanceValue& value,
				const XMLParser::MessageInterface& iface) // <ParameterInstance>
			{
				IMaterialParameterInstance* parameter = GetMaterialParameterInstance(name);
				if (!parameter)
				{
					String message = fmt::format("Cannot find Parameter Instance with name \"{0}\"", name);
					iface.SendError(message);
					return;
				}
				parameter->SetValue(value.Value);
			})

			.EndMaterialInstance() // </MaterialInstance>
			.Finalize(); // </IonAsset>

		return result.OK();
	}
}
