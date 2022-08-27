#pragma once

#include "Material.h"

namespace Ion
{
#pragma region Material Registry

	class ION_API MaterialRegistry
	{
	public:
		static std::shared_ptr<Material> QueryMaterial(Asset materialAsset);
		static std::shared_ptr<MaterialInstance> QueryMaterialInstance(Asset materialInstanceAsset);

	private:
		static MaterialRegistry& Get();

	private:
		THashMap<Asset, std::weak_ptr<Material>> m_Materials;
		THashMap<Asset, std::weak_ptr<MaterialInstance>> m_MaterialInstances;

		static MaterialRegistry* s_Instance;
	};

	inline MaterialRegistry& MaterialRegistry::Get()
	{
		return *(s_Instance ? s_Instance : s_Instance = new MaterialRegistry);
	}

#pragma endregion
}
