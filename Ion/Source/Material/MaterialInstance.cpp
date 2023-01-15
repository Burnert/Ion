#include "IonPCH.h"

#include "MaterialInstance.h"
#include "MaterialRegistry.h"

#include "RHI/UniformBuffer.h"

#include "Asset/AssetParser.h"

namespace Ion
{
#pragma region Material Parameter Instance

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
			m_TextureResource->Take([this](const TSharedPtr<TextureResource>& resource)
			{
				// Check to avoid overriding the texture when it's already been changed to a different one.
				if (resource == m_TextureResource)
					m_Texture = resource->GetRenderData().Texture;
			});
		}
	}

#pragma endregion

	Result<TSharedPtr<IAssetCustomData>, IOError> Ion::MaterialInstanceAssetType::Parse(const std::shared_ptr<XMLDocument>& xml) const
	{
		TSharedPtr<MaterialInstanceAssetData> data = MakeShared<MaterialInstanceAssetData>();

		XMLParserResult result = AssetParser(xml)
			.BeginAsset(AT_MaterialInstanceAssetType)
			.Begin(IASSET_NODE_MaterialInstance) // <MaterialInstance>
			.ParseCurrentAttributes(
				IASSET_ATTR_parent, [&, this](String parent)
				{
					data->ParentMaterialAssetVP = parent;
				}
			)
			.EnterEachNode(IASSET_NODE_MaterialInstance_ParameterInstance, [&, this](AssetParser& parser)
				{
					String paramName;
					EMaterialParameterType paramType = EMaterialParameterType::Null;

					parser.ParseCurrentEnumAttribute(IASSET_ATTR_type, paramType);
					parser.GetCurrentAttribute(IASSET_ATTR_name, paramName);

					MaterialInstanceAssetParameterInstanceValues values;
					parser.ParseCurrentNodeValue([&](String val) { values.Value = IMaterialParameter::ParseParamValue(val, paramType, parser); });

					data->Parameters.emplace_back(MaterialInstanceAssetData::Parameter { values, paramName, paramType });
				}
			)
			.End() // </MaterialInstance>
			.Finalize();

		if (!result.OK())
		{
			result.PrintMessages();
			ionthrow(IOError, result.GetFailMessage());
		}
		return data;
	}

	Result<void, IOError> MaterialInstanceAssetType::Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const
	{
		// @TODO: Make this work for binary archives too (not that trivial with xml)
		ionassert(ar.IsText(), "Binary archives are not supported at the moment.");
		ionassert(!inOutCustomData || inOutCustomData->GetType() == AT_MaterialInstanceAssetType);

		TSharedPtr<MaterialInstanceAssetData> data = inOutCustomData ? PtrCast<MaterialInstanceAssetData>(inOutCustomData) : MakeShared<MaterialInstanceAssetData>();

		XMLArchiveAdapter xmlAr = ar;
		ArchiveNode nodeRoot = ar.EnterRootNode();

		xmlAr.EnterNode(IASSET_NODE_MaterialInstance);
		ArchiveNode nodeMaterialInstance = ar.EnterNode(nodeRoot, "MaterialInstance", EArchiveNodeType::Map);

		ArchiveNode nodeParent = ar.EnterNode(nodeMaterialInstance, "Parent", EArchiveNodeType::Value);
		xmlAr.EnterAttribute(IASSET_ATTR_parent);
		nodeParent << data->ParentMaterialAssetVP;
		xmlAr.ExitAttribute(); // IASSET_ATTR_parent

		auto LSerializeParameterInstance = [&](MaterialInstanceAssetData::Parameter* param)
		{
			ArchiveNode node = ar.GetCurrentNode();

			ArchiveNode nodeType = ar.EnterNode(node, "Type", EArchiveNodeType::Value);
			xmlAr.EnterAttribute(IASSET_ATTR_type);
			nodeType << param->Type;
			xmlAr.ExitAttribute(); // IASSET_ATTR_type

			ArchiveNode nodeName = ar.EnterNode(node, "Name", EArchiveNodeType::Value);
			xmlAr.EnterAttribute(IASSET_ATTR_name);
			//String sAsset = ar.IsSaving() ? data->Description.Defaults.MaterialAssets[index]->GetVirtualPath() : EmptyString;
			nodeName << param->Name;
			xmlAr.ExitAttribute(); // IASSET_ATTR_name

			ArchiveNode nodeValue = ar.EnterAndUseNode(node, "Value", EArchiveNodeType::Value);
			IMaterialParameter::SerializeParamValue(ar, param->Type, param->Values.Value);
		};

		ArchiveNode nodeParameterInstances = ar.EnterNode(nodeMaterialInstance, "ParameterInstances", EArchiveNodeType::Seq);

		if (ar.IsLoading())
		{
			ON_YAML_AR(ar)
			{
				ArchiveNode nodeParameterInstance = ar.EnterAndUseNode(nodeParameterInstances, "", EArchiveNodeType::Map);
				for (; nodeParameterInstance; nodeParameterInstance = ar.EnterAndUseNextNode(nodeParameterInstance, EArchiveNodeType::Map))
					LSerializeParameterInstance(&data->Parameters.emplace_back());
			}
			else
			{
				for (bool b = xmlAr.TryEnterNode(IASSET_NODE_MaterialInstance_ParameterInstance); b || (xmlAr.ExitNode(), 0); b = xmlAr.TryEnterSiblingNode())
					LSerializeParameterInstance(&data->Parameters.emplace_back());
			}
		}
		else if (ar.IsSaving())
		{
			for (int32 i = 0; i < data->Parameters.size(); ++i)
			{
				ON_YAML_AR(ar)
				{
					ArchiveNode nodeParameterInstance = ar.EnterAndUseNode(nodeParameterInstances, "", EArchiveNodeType::Map);
					LSerializeParameterInstance(&data->Parameters[i]);
				}
				else
				{
					xmlAr.EnterNode(IASSET_NODE_MaterialInstance_ParameterInstance);
					LSerializeParameterInstance(&data->Parameters[i]);
					xmlAr.ExitNode(); // IASSET_NODE_MaterialInstance_ParameterInstance
				}
			}
		}

		xmlAr.ExitNode(); // IASSET_NODE_MaterialInstance

		inOutCustomData = data;

		return Ok();
	}

#pragma region Material Instance

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

		TRef<RHIUniformBufferDynamic> constants = m_ParentMaterial->m_MaterialConstants;
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

		TSharedPtr<MaterialInstanceAssetData> data = PtrCast<MaterialInstanceAssetData>(m_Asset->GetCustomData());
		ionassert(!data->ParentMaterialAssetVP.empty());

		if (auto result = Asset::Resolve(data->ParentMaterialAssetVP)
			.Err([&](Error& err) { MaterialLogger.Error("Could not set parent material to \"{}\".\n{}", data->ParentMaterialAssetVP, err.Message); }))
		{
			SetParentMaterial(MaterialRegistry::QueryMaterial(result.Unwrap()));

			for (MaterialInstanceAssetData::Parameter& param : data->Parameters)
			{
				IMaterialParameterInstance* parameter = GetMaterialParameterInstance(param.Name);
				if (!parameter)
				{
					MaterialLogger.Error("Cannot find Parameter Instance with name \"{}\"", param.Name);
					continue;
				}
				parameter->SetValue(param.Values.Value);
			}
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

#pragma endregion
}
