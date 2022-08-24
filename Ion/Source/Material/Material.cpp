#include "IonPCH.h"

#include "Material.h"
#include "RHI/Shader.h"
#include "RHI/UniformBuffer.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

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
				ionassert(std::holds_alternative<String>(def));

				MaterialParameterTexture2D* param = (MaterialParameterTexture2D*)this;
				String sValue = std::get<String>(def);
				Asset asset = Asset::Resolve(sValue)
					.Err([&](auto& err) { MaterialLogger.Error("Could not set Material Parameter default value to \"{}\".", sValue); })
					.UnwrapOr(Asset::None);
				param->SetDefaultValue(asset);
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
				ionassert(std::holds_alternative<String>(value));

				MaterialParameterInstanceTexture2D* param = (MaterialParameterInstanceTexture2D*)this;
				const String& sValue = std::get<String>(value);
				Asset asset = Asset::Resolve(sValue)
					.Err([&](auto& err) { MaterialLogger.Error("Could not set Material Parameter Instance value to \"{}\".", sValue); })
					.UnwrapOr(Asset::None);
				param->SetValue(asset);
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
		if (m_Value != (m_TextureResource ? m_TextureResource->GetAssetHandle() : Asset::None))
		{
			m_TextureResource = TextureResource::Query(m_Value);

			// @TODO: Make sure the instance is not deleted before this gets done
			m_TextureResource->Take([this](const TResourceRef<TextureResource>& resource)
			{
				// Check to avoid overriding the texture when it's already been changed to a different one.
				if (resource == m_TextureResource)
					m_Texture = resource->GetRenderData().Texture;
			});
		}
	}

	// End of MaterialParameterInstance impl ------------------------------------------------------------------------------------

	MaterialRegistry* MaterialRegistry::s_Instance = nullptr;

	std::shared_ptr<Material> MaterialRegistry::QueryMaterial(Asset materialAsset)
	{
		MaterialRegistry& instance = Get();

		std::shared_ptr<Material> material;

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

	std::shared_ptr<MaterialInstance> MaterialRegistry::QueryMaterialInstance(Asset materialInstanceAsset)
	{
		MaterialRegistry& instance = Get();

		std::shared_ptr<MaterialInstance> materialInstance;

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

	std::shared_ptr<Material> Material::Create()
	{
		return std::shared_ptr<Material>(new Material);
	}

	std::shared_ptr<Material> Material::CreateFromAsset(Asset materialAsset)
	{
		return std::shared_ptr<Material>(new Material(materialAsset));
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
				shaderPerm.Shader->Compile()
					.Ok([&queue, &material, &shaderUsage, &shaderPerm]
					{
						queue.PushMessage(FTaskMessage([material, shaderUsage, &shaderPerm]
						{
							ionassert(material->m_Shaders.find(shaderUsage) != material->m_Shaders.end(),
								"Shader usage had been removed before the shader was compiled.");
							shaderPerm.bCompiled = true;
						}));
					})
					.Err<ShaderCompilationError>([&](auto& err)
					{
						// @TODO: More info
						MaterialLogger.Error("Could not compile shader.");
					});
			});
			compileTask.Schedule();
		}
	}

	void Material::CompileShaders(const FOnShadersCompiled& onCompiled)
	{
		std::shared_ptr<ShadersCompiledCounter> counter = std::make_shared<ShadersCompiledCounter>((uint32)m_Shaders.size());

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
				shaderPerm.Shader->Compile()
					.Ok([&queue, &material, &shaderUsage, &shaderPerm, &counter, &onCompiled]
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
					})
					.Err<ShaderCompilationError>([&](auto& err)
					{
						// @TODO: More info
						MaterialLogger.Error("Could not compile shader.");
					});
			});
			compileTask.Schedule();
		}
	}

	bool Material::IsCompiled(EShaderUsage usage) const
	{
		if (m_Shaders.find(usage) == m_Shaders.end())
			return false;

		return m_Shaders.at(usage).bCompiled;
	}

	bool Material::BindShader(EShaderUsage usage) const
	{
		ionassert(m_Shaders.find(usage) != m_Shaders.end());

		if (!m_Shaders.at(usage).bCompiled)
			return false;
		
		m_Shaders.at(usage).Shader->Bind();

		return true;
	}

	const std::shared_ptr<RHIShader>& Material::GetShader(EShaderUsage usage) const
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
			MaterialLogger.Error("Cannot compile blank material code.");
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
			MaterialLogger.Error("Cannot compile blank material code.");
			return false;
		}

		if (m_Shaders.find(usage) == m_Shaders.end())
		{
			MaterialLogger.Error("Material cannot be compiled for the specified usage ({0}).", ToString(usage));
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
		if (!ParseAsset(materialAsset))
		{
			ionbreak("Could not parse Material asset.");
			return;
		}
		BuildConstantBuffer();
	}

	static TMaterialParameterTypeVariant _ParseParamValue(const String& val, EMaterialParameterType type, AssetParser& parser)
	{
		switch (type)
		{
			case EMaterialParameterType::Scalar:
			{
				TOptional<float> value = TStringParser<float>()(val);

				if (!value)
				{
					String message = fmt::format("Cannot parse the scalar parameter value string. \"{0}\" -> float", val);
					parser.GetInterface().SendError(message);
					return 0.0f;
				}
				return *value;
			}
			case EMaterialParameterType::Vector:
			{
				TOptional<Vector4> value = ParseVector4String(val.c_str());

				if (!value)
				{
					String message = fmt::format("Cannot parse the vector parameter value string. \"{0}\" -> Vector4", val);
					parser.GetInterface().SendError(message);
					return Vector4(0.0f);
				}
				return *value;
			}
			case EMaterialParameterType::Texture2D:
			{
				return val;
			}
		}
		parser.GetInterface().SendError("Tried to parse values of an invalid material parameter type.");
		return 0.0f;
	}

	bool Material::ParseAsset(Asset materialAsset)
	{
		ionassert(materialAsset);
		ionassert(materialAsset->GetType() == EAssetType::Material);

		return AssetParser(materialAsset)
			.BeginAsset(EAssetType::Material)
			.Begin(IASSET_NODE_Material) // <Material>
			.ParseAttributes(IASSET_NODE_Material_Code,
				IASSET_ATTR_source, [this](String source)
				{
					FilePath path = EnginePath::GetShadersPath() + "Material" + source;
					return LoadExternalMaterialCode(path);
				}
			)
			.EnterEachNode(IASSET_NODE_Material_Parameter, [this](AssetParser& parser)
			{
				String paramName;
				EMaterialParameterType paramType = EMaterialParameterType::Null;

				parser.ParseCurrentEnumAttribute(IASSET_ATTR_type, paramType);
				parser.GetCurrentAttribute(IASSET_ATTR_name, paramName);

				MaterialAssetParameterValues values;
				parser.TryParseNodeValue(IASSET_NODE_Material_Parameter_Default, [&](String def) { values.Default = _ParseParamValue(def, paramType, parser); });
				parser.TryParseNodeValue(IASSET_NODE_Material_Parameter_Min,     [&](String min) { values.Min =     _ParseParamValue(min, paramType, parser); });
				parser.TryParseNodeValue(IASSET_NODE_Material_Parameter_Max,     [&](String max) { values.Max =     _ParseParamValue(max, paramType, parser); });

				IMaterialParameter* parameter = AddParameter(paramName, paramType);
				parameter->SetValues(values.Default, values.Min, values.Max);
			})
			.End() // </Material>
			.Finalize()
			.OK();
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
			MaterialLogger.Error("Material Code is empty!");
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
		ionmatchresult(File::ReadToString(path),
			mcaseok source = R.Unwrap();
			melse return false;
		);

		SetMaterialCode(source);

		return true;
	}

	IMaterialParameter* Material::AddParameter(const String& name, EMaterialParameterType type)
	{
		auto it = m_Parameters.find(name);
		if (it != m_Parameters.end())
		{
			MaterialLogger.Error("Cannot add a material parameter, because a parameter with name \"{0}\" already exists.", name);
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
				MaterialLogger.Error("A Material cannot have more than 16 Texture Parameters.");
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
			MaterialLogger.Error("Cannot find a parameter with name \"{0}\".", name);
			return false;
		}

		ionassert(it->second, "MaterialParameter \"{0}\" is null.", name);

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

	std::shared_ptr<MaterialInstance> MaterialInstance::Create(const std::shared_ptr<Material>& parentMaterial)
	{
		return std::shared_ptr<MaterialInstance>(new MaterialInstance(parentMaterial));
	}

	std::shared_ptr<MaterialInstance> MaterialInstance::CreateFromAsset(Asset materialInstanceAsset)
	{
		return std::shared_ptr<MaterialInstance>(new MaterialInstance(materialInstanceAsset));
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

		std::shared_ptr<RHIUniformBufferDynamic> constants = m_ParentMaterial->m_MaterialConstants;
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

	MaterialInstance::MaterialInstance(const std::shared_ptr<Material>& parentMaterial) :
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

	void MaterialInstance::SetParentMaterial(const std::shared_ptr<Material>& material)
	{
		DestroyParameterInstances();
		m_ParentMaterial = material;
		CreateParameterInstances();
	}

	void MaterialInstance::CreateParameterInstances()
	{
		ionassert(m_ParameterInstances.empty(), "Destroy existing instances before creating new ones.");

		ionassert(m_ParentMaterial, "Cannot create parameter instances. Parent Material is not set.");

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

		return AssetParser(materialInstanceAsset)
			.BeginAsset(EAssetType::MaterialInstance)
			.Begin(IASSET_NODE_MaterialInstance) // <MaterialInstance>
			.ParseCurrentAttributes(
				IASSET_ATTR_parent, [this](String parent)
				{
					Asset asset = Asset::Resolve(parent)
						.Err([&](auto& err) { MaterialLogger.Error("Could not set parent material to \"{}\"", parent); })
						.UnwrapOr(Asset::None);
					SetParentMaterial(MaterialRegistry::QueryMaterial(asset));
				}
			)
			.EnterEachNode(IASSET_NODE_MaterialInstance_ParameterInstance, [this](AssetParser& parser)
				{
					String paramName;
					EMaterialParameterType paramType = EMaterialParameterType::Null;

					parser.ParseCurrentEnumAttribute(IASSET_ATTR_type, paramType);
					parser.GetCurrentAttribute(IASSET_ATTR_name, paramName);

					MaterialInstanceAssetParameterInstanceValues values;
					parser.ParseCurrentNodeValue([&](String val) { values.Value = _ParseParamValue(val, paramType, parser); });

					IMaterialParameterInstance* parameter = GetMaterialParameterInstance(paramName);
					if (!parameter)
					{
						String message = fmt::format("Cannot find Parameter Instance with name \"{0}\"", paramName);
						parser.GetInterface().SendError(message);
						return;
					}
					parameter->SetValue(values.Value);
				}
			)
			.End() // </MaterialInstance>
			.Finalize()
			.OK();
	}
}
