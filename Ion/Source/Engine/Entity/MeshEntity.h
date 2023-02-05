#pragma once

#include "EntityOld.h"
#include "Engine/Components/MeshComponent.h"
#include "Asset/AssetCommon.h"

namespace Ion
{
	class ION_API MeshEntity : public EntityOld
	{
	public:
		MCLASS(MeshEntity)
		using Super = EntityOld;

		MeshEntity();

		MeshComponent* GetMeshComponent() const;

		void SetMeshFromAsset(const Asset& asset);

		void SetMesh(const std::shared_ptr<Mesh>& mesh);
		std::shared_ptr<Mesh> GetMesh() const;

		// EntityOld overrides

		virtual void Serialize(Archive& ar) override;

		virtual EntityOld* Duplicate_Internal() const override;

	protected:
		virtual void OnSpawn(World* worldContext) override;
		virtual void OnDestroy() override;

		// End of EntityOld overrides
	};
}
