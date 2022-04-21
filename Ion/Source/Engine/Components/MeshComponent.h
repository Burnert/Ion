#pragma once

#include "SceneComponent.h"

#include "Core/Asset/Asset.h"

#include "Resource/MeshResource.h"

namespace Ion
{
	ENTITY_COMPONENT_CLASS_HEADER(MeshComponent);

	class ION_API MeshComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(MeshComponent, "Mesh")

		void SERIALCALL BuildRendererData(RRendererData& data);

		void SetMeshFromAsset(const Asset& asset);

		void SetMesh(const TShared<Mesh>& mesh);
		TShared<Mesh> GetMesh() const;

		RPrimitiveRenderProxy AsRenderProxy() const;

	private:
		MeshComponent();

	private:
		Asset m_MeshAsset;
		TShared<MeshResource> m_MeshResource;
		TShared<Mesh> m_Mesh;
	};
}
