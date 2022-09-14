#include "IonPCH.h"

#include "MeshEntity.h"
#include "Renderer/Mesh.h"
#include "Engine/World.h"

namespace Ion
{
	MeshEntity::MeshEntity()
	{
		SetNoCreateRootOnSpawn();
		SetName("Mesh");
		m_ClassName = "MeshEntity";
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

	void MeshEntity::SetMesh(const std::shared_ptr<Mesh>& mesh)
	{
		GetMeshComponent()->SetMesh(mesh);
	}

	std::shared_ptr<Mesh> MeshEntity::GetMesh() const
	{
		return GetMeshComponent()->GetMesh();
	}

	void MeshEntity::Serialize(Archive& ar)
	{
		Super::Serialize(ar);

		XMLArchiveAdapter xmlAr = ar;

		xmlAr.EnterNode("Mesh");
		String vp = /*ar.IsSaving() ? GetMeshComponent()->GetMesh()->GetMeshResource()->GetAssetHandle()->GetVirtualPath() :*/ EmptyString;
		xmlAr << vp;
		xmlAr.ExitNode(); // "Mesh"

		if (ar.IsLoading())
		{
			// @TODO: This can't be done here because the world context is not set at this point.

			//Asset::Resolve(vp)
			//	.Ok([&](const Asset& asset)
			//	{
			//		TSharedPtr<MeshResource> resource = MeshResource::Query(asset);
			//		std::shared_ptr<Mesh> mesh = Mesh::CreateFromResource(resource);
			//		SetMesh(mesh);
			//	});
		}
	}

	Entity* MeshEntity::Duplicate_Internal() const
	{
		return new MeshEntity(*this);
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
