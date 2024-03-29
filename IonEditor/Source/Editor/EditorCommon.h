#pragma once

#include "Engine/Components/ComponentOld.h"
#include "Asset/Asset.h"

namespace Ion::Editor
{
#define DNDID_WorldTreeNode        "Ion_DND_WorldTreeNode"

#define DNDID_InsertEntity         "Ion_DND_InsertEntity"
#define DNDID_InsertComponent      "Ion_DND_InsertComponent"
#define DNDID_InsertSceneComponent "Ion_DND_InsertSceneComponent"

#define DNDID_MeshAsset            "Ion_DND_MeshAsset"

	struct DNDInsertEntityData
	{
		using InstantiateFunc = EntityOld*(World*, void*);
		InstantiateFunc* Instantiate;
		void* CustomData = nullptr;
	};

	struct DNDInsertComponentData
	{
		using InstantiateFunc = ComponentOld*(World*, ComponentTypeID);
		InstantiateFunc* Instantiate;
		ComponentTypeID ID;
	};

	struct DNDAssetData
	{
		Asset AssetHandle;
	};
}
