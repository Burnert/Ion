#pragma once

#include "Entity.h"
#include "Engine/Components/MeshComponent.h"
#include "Asset/AssetCommon.h"

namespace Ion
{
	class ION_API MeshEntity : public Entity
	{
	public:
		MeshEntity();
		MeshEntity(const GUID& guid);

		MeshComponent* GetMeshComponent() const;

		void SetMeshFromAsset(const Asset& asset);

		void SetMesh(const std::shared_ptr<Mesh>& mesh);
		std::shared_ptr<Mesh> GetMesh() const;

		// Entity overrides

		virtual void Serialize(Archive& ar) override;

		virtual Entity* Duplicate_Internal() const override;

	protected:
		virtual void OnSpawn(World* worldContext) override;
		virtual void OnDestroy() override;

		// End of Entity overrides
	};
}
