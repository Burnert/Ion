#pragma once

#include "Component.h"

namespace Ion
{
	class ION_API MeshComponent : public Component
	{
		ENTITY_COMPONENT_CLASS_BODY(Component)

		MeshComponent();

		// Component Callback methods

		void OnCreate();
		void OnDestroy();
		void Tick(float deltaTime);

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

	private:
		void NotifyTransformUpdated();

	private:
		Transform m_Transform;
		TShared<Mesh> m_Mesh;
	};
}
