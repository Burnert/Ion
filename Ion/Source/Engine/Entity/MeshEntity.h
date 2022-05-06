#pragma once

#include "Entity.h"
#include "Engine/Components/MeshComponent.h"
#include "Core/Asset/AssetCommon.h"

namespace Ion
{
	class ION_API MeshEntity : public Entity
	{
	public:
		MeshEntity();
		MeshEntity(const GUID& guid);

		MeshComponent* GetMeshComponent() const;

		void SetMeshFromAsset(const Asset& asset);

		void SetMesh(const TShared<Mesh>& mesh);
		TShared<Mesh> GetMesh() const;

		// Entity overrides

		virtual Entity* Duplicate_Internal() const override;

	protected:
		virtual void OnSpawn(World* worldContext) override;
		virtual void OnDestroy() override;

		// End of Entity overrides
	};
}
