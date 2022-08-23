#pragma once

#include "SceneComponent.h"

#include "Asset/Asset.h"

#include "Resource/MeshResource.h"

namespace Ion
{
	ENTITY_COMPONENT_CLASS_HEADER(MeshComponent);

	class ION_API MeshComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(MeshComponent, "Mesh")

		virtual void OnCreate() override;

		void SERIALCALL BuildRendererData(RRendererData& data);

		void SetMeshFromAsset(const Asset& asset);

		void SetMeshAsset(const Asset& asset);
		Asset GetMeshAsset() const;

		void SetMeshResource(const TResourceRef<MeshResource>& resource);
		Asset GetMeshResource() const;

		void SetMesh(const std::shared_ptr<Mesh>& mesh);
		std::shared_ptr<Mesh> GetMesh() const;

		RPrimitiveRenderProxy AsRenderProxy() const;

	private:
		Asset m_MeshAsset;
		TResourceRef<MeshResource> m_MeshResource;
		std::shared_ptr<Mesh> m_Mesh;
	};

	inline Asset MeshComponent::GetMeshAsset() const
	{
		return m_MeshAsset;
	}

	inline Asset MeshComponent::GetMeshResource() const
	{
		return Asset();
	}
}
