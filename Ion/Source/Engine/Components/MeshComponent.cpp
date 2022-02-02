#include "IonPCH.h"

#include "MeshComponent.h"
#include "Engine/World.h"
#include "Renderer/Mesh.h"
#include "Renderer/Scene.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(MeshComponent)

	ENTITY_COMPONENT_CALLBACK_ONCREATE_FUNC(MeshComponent)
	ENTITY_COMPONENT_CALLBACK_ONDESTROY_FUNC(MeshComponent)
	//ENTITY_COMPONENT_CALLBACK_TICK_FUNC(MeshComponent)

	MeshComponent::MeshComponent()
	{
		SetTickEnabled(false);
		InitAsSceneComponent();
	}

	void MeshComponent::OnCreate()
	{
		Component::OnCreate();
		LOG_INFO("MeshComponent::OnCreate()");
	}

	void MeshComponent::OnDestroy()
	{
		Component::OnDestroy();
		LOG_INFO("MeshComponent::OnDestroy()");
	}

	void MeshComponent::Tick(float deltaTime)
	{
		Component::Tick(deltaTime);
		LOG_INFO("MeshComponent::Tick({0})", deltaTime);
	}

	void MeshComponent::SetMesh(const TShared<Mesh>& mesh)
	{
		// @TODO: Temporary - change how Scene works
		// It has to take in a Transform with a Mesh
		// A Mesh by itself should not have a transform
		// because it will be shared between different components
		if (mesh != m_Mesh)
		{
			// TEMPORARY
			mesh->SetTransform(m_Transform.GetMatrix());

			Scene* scene = GetWorldContext()->GetScene();
			scene->RemoveDrawableObject(m_Mesh.get());
			scene->AddDrawableObject(mesh.get());
		}
		m_Mesh = mesh;
	}

	TShared<Mesh> MeshComponent::GetMesh() const
	{
		return m_Mesh;
	}

	void MeshComponent::SetTransform(const Transform& transform)
	{
		m_Transform = transform;
		NotifyTransformUpdated();
	}

	const Transform& MeshComponent::GetTransform() const
	{
		return m_Transform;
	}

	void MeshComponent::SetLocation(const Vector3& location)
	{
		m_Transform.SetLocation(location);
		NotifyTransformUpdated();
	}

	const Vector3& MeshComponent::GetLocation() const
	{
		return m_Transform.GetLocation();
	}

	void MeshComponent::SetRotation(const Rotator& rotation)
	{
		m_Transform.SetRotation(rotation);
		NotifyTransformUpdated();
	}

	const Rotator& MeshComponent::GetRotation() const
	{
		return m_Transform.GetRotation();
	}

	void MeshComponent::SetScale(const Vector3& scale)
	{
		m_Transform.SetScale(scale);
		NotifyTransformUpdated();
	}

	const Vector3& MeshComponent::GetScale() const
	{
		return m_Transform.GetScale();
	}

	void MeshComponent::NotifyTransformUpdated()
	{
		// TEMPORARY
		m_Mesh->SetTransform(m_Transform.GetMatrix());
	}
}
