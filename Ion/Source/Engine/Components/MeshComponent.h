#pragma once

#include "SceneComponent.h"

namespace Ion
{
	class ION_API MeshComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY()

		// Component Callback methods

		void COMPCALLBACKFUNC OnCreate();
		void COMPCALLBACKFUNC OnDestroy();
		void COMPCALLBACKFUNC BuildRendererData(RRendererData& data);
		//void Tick(float deltaTime);

		// End of Component Callback methods

		void SetMesh(const TShared<Mesh>& mesh);
		TShared<Mesh> GetMesh() const;

	private:
		MeshComponent();

	private:
		SceneComponentData m_SceneData;
		TShared<Mesh> m_Mesh;
	};

	template<>
	struct ComponentTypeDefaults<MeshComponent>
	{
		static constexpr const char* Name = "Mesh Component";
	};
}
