#include "IonPCH.h"

#include "MeshComponent.h"
#include "Engine/World.h"
#include "Engine/Entity.h"
#include "Renderer/Renderer.h"

#pragma warning(disable:26815)

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
		Transform worldTransform = GetWorldTransform();

		RPrimitiveRenderProxy mesh;
		mesh.Transform     = worldTransform.GetMatrix();
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
			//mesh->SetTransform(m_SceneData.Transform.GetMatrix());

			//Scene* scene = GetWorldContext()->GetEditorScene();
			//scene->RemoveDrawableObject(m_Mesh.get());
			//scene->AddDrawableObject(mesh.get());
		}
		m_Mesh = mesh;
	}

	TShared<Mesh> MeshComponent::GetMesh() const
	{
		return m_Mesh;
	}
}
