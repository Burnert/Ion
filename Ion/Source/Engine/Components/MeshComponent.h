#pragma once

#include "SceneComponent.h"

namespace Ion
{
	class ION_API MeshComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(MeshComponent, "Mesh")

		void SERIALCALL BuildRendererData(RRendererData& data);

		void SetMesh(const TShared<Mesh>& mesh);
		TShared<Mesh> GetMesh() const;

		RPrimitiveRenderProxy AsRenderProxy() const;

	private:
		MeshComponent();

	private:
		TShared<Mesh> m_Mesh;
	};
}
