#include "IonPCH.h"

#include "MeshComponent.h"
#include "Engine/World.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(MeshComponent)

	ENTITY_COMPONENT_STATIC_CALLBACK_ONCREATE_FUNC()
	ENTITY_COMPONENT_STATIC_CALLBACK_ONDESTROY_FUNC()
	ENTITY_COMPONENT_STATIC_CALLBACK_BUILDRENDERERDATA_FUNC()
		//ENTITY_COMPONENT_STATIC_CALLBACK_TICK_FUNC()

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

	void MeshComponent::BuildRendererData(RRendererData& data)
	{
		Material* material = m_Mesh->GetMaterial().lock().get();

		RPrimitiveRenderProxy mesh;
		mesh.Transform     = m_SceneData.Transform.GetMatrix();
		mesh.Material      = material;
		mesh.Shader        = material->GetShader().get();
		mesh.VertexBuffer  = m_Mesh->GetVertexBufferRaw();
		mesh.IndexBuffer   = m_Mesh->GetIndexBufferRaw();
		mesh.UniformBuffer = m_Mesh->GetUniformBufferRaw();
		data.AddPrimitive(mesh);
	}

	//void MeshComponent::Tick(float deltaTime)
	//{
	//	Component::Tick(deltaTime);
	//	LOG_INFO("MeshComponent::Tick({0})", deltaTime);
	//}

	void MeshComponent::SetMesh(const TShared<Mesh>& mesh)
	{
		// @TODO: Temporary - change how Scene works
		// It has to take in a Transform with a Mesh
		// A Mesh by itself should not have a transform
		// because it will be shared between different components
		if (mesh != m_Mesh)
		{
			// TEMPORARY
			mesh->SetTransform(m_SceneData.Transform.GetMatrix());

			//Scene* scene = GetWorldContext()->GetScene();
			//scene->RemoveDrawableObject(m_Mesh.get());
			//scene->AddDrawableObject(mesh.get());
		}
		m_Mesh = mesh;
	}

	TShared<Mesh> MeshComponent::GetMesh() const
	{
		return m_Mesh;
	}

	void MeshComponent::SetTransform(const Transform& transform)
	{
		m_SceneData.Transform = transform;
		NotifyTransformUpdated();
	}

	const Transform& MeshComponent::GetTransform() const
	{
		return m_SceneData.Transform;
	}

	void MeshComponent::SetLocation(const Vector3& location)
	{
		m_SceneData.Transform.SetLocation(location);
		NotifyTransformUpdated();
	}

	const Vector3& MeshComponent::GetLocation() const
	{
		return m_SceneData.Transform.GetLocation();
	}

	void MeshComponent::SetRotation(const Rotator& rotation)
	{
		m_SceneData.Transform.SetRotation(rotation);
		NotifyTransformUpdated();
	}

	const Rotator& MeshComponent::GetRotation() const
	{
		return m_SceneData.Transform.GetRotation();
	}

	void MeshComponent::SetScale(const Vector3& scale)
	{
		m_SceneData.Transform.SetScale(scale);
		NotifyTransformUpdated();
	}

	const Vector3& MeshComponent::GetScale() const
	{
		return m_SceneData.Transform.GetScale();
	}

	void MeshComponent::SetVisible(bool bVisible)
	{
		m_SceneData.bVisible = bVisible;
	}

	bool MeshComponent::IsVisible() const
	{
		return m_SceneData.bVisible;
	}

	void MeshComponent::SetVisibleInGame(bool bVisibleInGame)
	{
		m_SceneData.bVisibleInGame = bVisibleInGame;
	}

	bool MeshComponent::IsVisibleInGame() const
	{
		return m_SceneData.bVisibleInGame;
	}

	void MeshComponent::NotifyTransformUpdated()
	{
		// TEMPORARY
		m_Mesh->SetTransform(m_SceneData.Transform.GetMatrix());
	}
}
