#include "IonPCH.h"

#include "Material.h"

#include "RHI/RHI.h"
#include "RHI/Shader.h"
#include "RHI/UniformBuffer.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetParser.h"

namespace Ion
{
	ShaderPermutation::ShaderPermutation(EShaderUsage usage) :
		Shader(RHIShader::Create()),
		Usage(usage),
		bCompiled(false)
	{
	}

#pragma region Material Parameter

	TMaterialParameterTypeVariant IMaterialParameter::ParseParamValue(const String& val, EMaterialParameterType type, AssetParser& parser)
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

	TMaterialParameterTypeVariant IMaterialParameter::ParseParamValue(const String& val, EMaterialParameterType type)
	{
		switch (type)
		{
		case EMaterialParameterType::Scalar:
		{
			TOptional<float> value = TStringParser<float>()(val);

			if (!value)
			{
				//String message = fmt::format("Cannot parse the scalar parameter value string. \"{0}\" -> float", val);
				//parser.GetInterface().SendError(message);
				return 0.0f;
			}
			return *value;
		}
		case EMaterialParameterType::Vector:
		{
			TOptional<Vector4> value = ParseVector4String(val.c_str());

			if (!value)
			{
				//String message = fmt::format("Cannot parse the vector parameter value string. \"{0}\" -> Vector4", val);
				//parser.GetInterface().SendError(message);
				return Vector4(0.0f);
			}
			return *value;
		}
		case EMaterialParameterType::Texture2D:
		{
			return val;
		}
		}
		//parser.GetInterface().SendError("Tried to parse values of an invalid material parameter type.");
		return 0.0f;
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

#pragma endregion

	Result<TSharedPtr<IAssetCustomData>, IOError> MaterialAssetType::Parse(const std::shared_ptr<XMLDocument>& xml) const
	{
		TSharedPtr<MaterialAssetData> data = MakeShared<MaterialAssetData>();

		XMLParserResult result = AssetParser(xml)
			.BeginAsset(AT_MaterialAssetType)
			.Begin(IASSET_NODE_Material) // <Material>
			.ParseAttributes(IASSET_NODE_Material_Code,
				IASSET_ATTR_source, [&, this](String source)
				{
					data->MaterialShaderCodePath = RHI::GetEngineShadersPath() + "Material" + source;
				}
			)
			.EnterEachNode(IASSET_NODE_Material_Parameter, [&, this](AssetParser& parser)
			{
				String paramName;
				EMaterialParameterType paramType = EMaterialParameterType::Null;

				parser.ParseCurrentEnumAttribute(IASSET_ATTR_type, paramType);
				parser.GetCurrentAttribute(IASSET_ATTR_name, paramName);

				MaterialAssetParameterValues values;
				parser.TryParseNodeValue(IASSET_NODE_Material_Parameter_Default, [&](String def) { values.Default = IMaterialParameter::ParseParamValue(def, paramType, parser); });
				parser.TryParseNodeValue(IASSET_NODE_Material_Parameter_Min,     [&](String min) { values.Min =     IMaterialParameter::ParseParamValue(min, paramType, parser); });
				parser.TryParseNodeValue(IASSET_NODE_Material_Parameter_Max,     [&](String max) { values.Max =     IMaterialParameter::ParseParamValue(max, paramType, parser); });

				data->Parameters.emplace_back(MaterialAssetData::Parameter { values, paramName, paramType, });
			})
			.End() // </Material>
			.Finalize();

		if (!result.OK())
		{
			result.PrintMessages();
			ionthrow(IOError, result.GetFailMessage());
		}
		return data;
	}

	Result<void, IOError> MaterialAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");
		ionassert(!inOutCustomData || inOutCustomData->GetType() == AT_MaterialAssetType);

		TSharedPtr<MaterialAssetData> data = inOutCustomData ? PtrCast<MaterialAssetData>(inOutCustomData) : MakeShared<MaterialAssetData>();

		XMLArchiveAdapter xmlAr = ar;

		xmlAr.EnterNode(IASSET_NODE_Material);

		xmlAr.EnterNode(IASSET_NODE_Material_Code);

		xmlAr.EnterAttribute(IASSET_ATTR_source);
		String sSource = ar.IsSaving() ?
			data->MaterialShaderCodePath :
			EmptyString;
		xmlAr << sSource;
		if (ar.IsLoading())
			data->MaterialShaderCodePath = RHI::GetEngineShadersPath() / "Material" / sSource;
		xmlAr.ExitAttribute(); // IASSET_ATTR_source

		xmlAr.ExitNode(); // IASSET_NODE_Material_Code

		auto LSerializeParameter = [&](MaterialAssetData::Parameter* param)
		{
			xmlAr.EnterAttribute(IASSET_ATTR_type);
			xmlAr << param->Type;
			xmlAr.ExitAttribute(); // IASSET_ATTR_type

			xmlAr.EnterAttribute(IASSET_ATTR_name);
			//String sAsset = ar.IsSaving() ? data->Description.Defaults.MaterialAssets[index]->GetVirtualPath() : EmptyString;
			xmlAr << param->Name;
			xmlAr.ExitAttribute(); // IASSET_ATTR_name

			if (xmlAr.TryEnterNode(IASSET_NODE_Material_Parameter_Default))
			{
				String def; // @TODO: Save
				xmlAr << def;
				if (ar.IsLoading())
					param->Values.Default = IMaterialParameter::ParseParamValue(def, param->Type);
				xmlAr.ExitNode();
			}
			if (xmlAr.TryEnterNode(IASSET_NODE_Material_Parameter_Min))
			{
				String min; // @TODO: Save
				xmlAr << min;
				if (ar.IsLoading())
					param->Values.Min = IMaterialParameter::ParseParamValue(min, param->Type);
				xmlAr.ExitNode();
			}
			if (xmlAr.TryEnterNode(IASSET_NODE_Material_Parameter_Max))
			{
				String max; // @TODO: Save
				xmlAr << max;
				if (ar.IsLoading())
					param->Values.Max = IMaterialParameter::ParseParamValue(max, param->Type);
				xmlAr.ExitNode();
			}
		};

		if (ar.IsLoading())
		{
			for (bool b = xmlAr.TryEnterNode(IASSET_NODE_Material_Parameter); b || (xmlAr.ExitNode(), 0); b = xmlAr.TryEnterSiblingNode())
				LSerializeParameter(&data->Parameters.emplace_back());
		}
		else if (ar.IsSaving())
		{
			for (int32 i = 0; i < data->Parameters.size(); ++i)
			{
				xmlAr.EnterNode(IASSET_NODE_Material_Parameter);
				LSerializeParameter(&data->Parameters[i]);
				xmlAr.ExitNode(); // IASSET_NODE_Material_Parameter
			}
		}

		xmlAr.ExitNode(); // IASSET_NODE_Material

		inOutCustomData = data;

		return Ok();
	}

#pragma region Material

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

			shaderPerm.Shader->AddShaderSource(EShaderType::Vertex, m_MaterialCode, m_MaterialShaderPath);
			shaderPerm.Shader->AddShaderSource(EShaderType::Pixel, m_MaterialCode, m_MaterialShaderPath);

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

			shaderPerm.Shader->AddShaderSource(EShaderType::Vertex, m_MaterialCode, m_MaterialShaderPath);
			shaderPerm.Shader->AddShaderSource(EShaderType::Pixel, m_MaterialCode, m_MaterialShaderPath);

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

	const TRef<RHIShader>& Material::GetShader(EShaderUsage usage) const
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
		TSharedPtr<MaterialAssetData> data = PtrCast<MaterialAssetData>(m_Asset->GetCustomData());
		ionassert(data->MaterialShaderCodePath.IsFile());

		LoadExternalMaterialCode(data->MaterialShaderCodePath);

		for (const MaterialAssetData::Parameter& param : data->Parameters)
		{
			IMaterialParameter* parameter = AddParameter(param.Name, param.Type);
			parameter->SetValues(param.Values.Default, param.Values.Min, param.Values.Max);
		}
		BuildConstantBuffer();
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

		m_MaterialShaderPath = path;
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

#pragma endregion
}
