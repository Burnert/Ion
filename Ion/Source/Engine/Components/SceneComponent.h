#pragma once

#include "Component.h"

namespace Ion
{
	class ION_API SceneComponent : public Component
	{
	public:
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

		Transform GetWorldTransform() const;

	protected:
		SceneComponent();

	private:
		SceneComponentData m_SceneData;
	};
}
