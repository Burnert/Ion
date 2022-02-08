#pragma once

#include "Component.h"

namespace Ion
{
	class ION_API MeshComponent : public Component
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

		void SetTransform(const Transform& transform);
		const Transform& GetTransform() const;

		void SetLocation(const Vector3& location);
		const Vector3& GetLocation() const;

		void SetRotation(const Rotator& rotation);
		const Rotator& GetRotation() const;

		void SetScale(const Vector3& scale);
		const Vector3& GetScale() const;

		void SetVisible(bool bVisible);
		bool IsVisible() const;

		void SetVisibleInGame(bool bVisibleInGame);
		bool IsVisibleInGame() const;

	private:
		void NotifyTransformUpdated();

		MeshComponent();

	private:
		SceneComponentData m_SceneData;
		TShared<Mesh> m_Mesh;
	};
}