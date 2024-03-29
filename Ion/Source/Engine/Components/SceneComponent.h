#pragma once

#include "ComponentOld.h"
#include "Component.h"
#include "Engine/SceneObjectData.h"

namespace Ion
{
#pragma region Old Scene Component

	/* Abstract class */
	class ION_API SceneComponent : public ComponentOld
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

		const Transform& GetWorldTransform() const;

		bool ShouldBeRendered() const;

		/* Attaches to the specified parent component.
		   Sets the owner (entity) of this component to the parent's owner.
		   Updates the world transform cache. */
		void AttachTo(SceneComponent* parent);
		/* Detaches from the parent and removes from the owner.
		   Updates the world transform cache.
		   Returns this component */
		SceneComponent* Detach();
		SceneComponent* GetParent() const;
		bool HasChildren() const;
		const TArray<SceneComponent*>& GetChildren() const;
		TArray<SceneComponent*> GetAllDescendants() const;

		SceneComponent* DeepDuplicate() const;

	protected:
		SceneComponent();

	private:
		void AddChild(SceneComponent* component);
		void RemoveChild(SceneComponent* component);
		void GetAllDescendants_Internal(TArray<SceneComponent*>& outArray) const;

		/* Called in any function that changes the relative transform. */
		void UpdateWorldTransformCache();
		void UpdateChildrenWorldTransformCache();

	private:
		Transform m_RelativeTransform;
		Transform m_WorldTransformCache;

		SceneObjectData m_SceneData;

		SceneComponent* m_Parent;
		TArray<SceneComponent*> m_Children;

		friend class EntityOld;
	};

	ENTITY_COMPONENT_CLASS_HEADER(EmptySceneComponent);

	class EmptySceneComponent final : public SceneComponent
	{
		ENTITY_COMPONENT_CLASS_BODY(EmptySceneComponent, "Empty Scene Component")

	private:
		EmptySceneComponent();
	};

	inline const Transform& SceneComponent::GetTransform() const
	{
		return m_RelativeTransform;
	}

	inline const Vector3& SceneComponent::GetLocation() const
	{
		return m_RelativeTransform.GetLocation();
	}

	inline const Rotator& SceneComponent::GetRotation() const
	{
		return m_RelativeTransform.GetRotation();
	}

	inline const Vector3& SceneComponent::GetScale() const
	{
		return m_RelativeTransform.GetScale();
	}

	inline bool SceneComponent::IsVisible() const
	{
		return m_SceneData.bVisible;
	}

	inline bool SceneComponent::IsVisibleInGame() const
	{
		return m_SceneData.bVisibleInGame;
	}

	inline const Transform& SceneComponent::GetWorldTransform() const
	{
		return m_WorldTransformCache;
	}

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

#pragma endregion

	class ION_API MSceneComponent : public MComponent
	{
	public:
		MCLASS(MSceneComponent)
		using Super = MComponent;

	public:
		MSceneComponent();

		void Attach(const TObjectPtr<MSceneComponent>& component);
		MMETHOD(Attach, const TObjectPtr<MSceneComponent>&)

		const TArray<TObjectPtr<MSceneComponent>> GetChildComponents() const;
		MMETHOD(GetChildComponents)

		TArray<TObjectPtr<MSceneComponent>> GetAllChildren() const;
		MMETHOD(GetAllChildren)

	protected:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void Tick(float deltaTime) override;

	private:
		void GetAllChildren_Internal(TArray<TObjectPtr<MSceneComponent>>& outChildren) const;

	public:
		// @TODO: Make transform reflectable
		Transform Transform;
		//MFIELD(m_Transform)

	private:
		TArray<TObjectPtr<MSceneComponent>> m_ChildComponents;
		MFIELD(m_ChildComponents)

	public:
		bool bVisible;
		MFIELD(bVisible)

		bool bVisibleInGame;
		MFIELD(bVisibleInGame)
	};

	FORCEINLINE const TArray<TObjectPtr<MSceneComponent>> MSceneComponent::GetChildComponents() const
	{
		return m_ChildComponents;
	}
}
