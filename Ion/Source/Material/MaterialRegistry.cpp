#include "IonPCH.h"

#include "MaterialRegistry.h"
#include "MaterialInstance.h"

namespace Ion
{
#pragma region Material Registry

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

#pragma endregion
}
