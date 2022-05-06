#include "IonPCH.h"

#include "MeshEntity.h"
#include "Engine/World.h"

namespace Ion
{
	MeshEntity::MeshEntity() :
		MeshEntity(GUID())
	{
	}

	MeshEntity::MeshEntity(const GUID& guid) :
		Entity(guid)
	{
		SetNoCreateRootOnSpawn();
		SetName("Mesh");
	}

	MeshComponent* MeshEntity::GetMeshComponent() const
	{
		ionassert(GetRootComponent()->GetClassName() == "MeshComponent",
			"The root component has been set to a wrong type.");
		return (MeshComponent*)GetRootComponent();
	}

	void MeshEntity::SetMeshFromAsset(const Asset& asset)
	{
		GetMeshComponent()->SetMeshFromAsset(asset);
	}

	void MeshEntity::SetMesh(const TShared<Mesh>& mesh)
	{
		GetMeshComponent()->SetMesh(mesh);
	}

	TShared<Mesh> MeshEntity::GetMesh() const
	{
		return GetMeshComponent()->GetMesh();
	}

	void MeshEntity::OnSpawn(World* worldContext)
	{
		Entity::OnSpawn(worldContext);

		ComponentRegistry& registry = worldContext->GetComponentRegistry();
		SetRootComponent(registry.CreateComponent<MeshComponent>());
		GetRootComponent()->SetName("Mesh");
	}

	void MeshEntity::OnDestroy()
	{
		Entity::OnDestroy();
	}
}
