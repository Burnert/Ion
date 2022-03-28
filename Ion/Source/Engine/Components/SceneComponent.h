#pragma once

#include "Component.h"

namespace Ion
{
	struct SceneComponentData
	{
		Transform Transform;
		uint8 bVisible : 1;
		uint8 bVisibleInGame : 1;

		SceneComponentData() :
			bVisible(true),
			bVisibleInGame(true)
		{
		}
	};

	/* Abstract class */
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

		void AttachTo(SceneComponent* parent);
		/* Returns this component */
		SceneComponent* Detach();
		SceneComponent* GetParent() const;
		bool HasChildren() const;
		const TArray<SceneComponent*>& GetChildren() const;
		TArray<SceneComponent*> GetAllDescendants() const;

	protected:
		SceneComponent();

	private:
		void AddChild(SceneComponent* component);
		void RemoveChild(SceneComponent* component);
		void GetAllDescendants_Internal(TArray<SceneComponent*>& outArray) const;

	private:
		SceneComponentData m_SceneData;

		SceneComponent* m_Parent;
		TArray<SceneComponent*> m_Children;
	};

	class EmptySceneComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(EmptySceneComponent, "Empty Scene Component")

	private:
		EmptySceneComponent();
	};

	inline SceneComponent* SceneComponent::GetParent() const
	{
		return m_Parent;
	}

	inline bool SceneComponent::HasChildren() const
	{
		return !m_Children.empty();
	}

	inline const TArray<SceneComponent*>& SceneComponent::GetChildren() const
	{
		return m_Children;
	}
}
