#include "IonPCH.h"

#include "MeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/Entity/Entity.h"
#include "Renderer/Renderer.h"

#pragma warning(disable:26815)

namespace Ion
{
	DECLARE_ENTITY_COMPONENT_CLASS(MeshComponent)

	DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA()

	MeshComponent::MeshComponent()
	{
		SetTickEnabled(false);
	}

	void MeshComponent::BuildRendererData(RRendererData& data)
	{
		// @TODO: Takes 1.5-2ms for 10k components

		ionassert(GetOwner());

		if (!m_Mesh)
			return;

		// @TODO: Make a check to see if the mesh can be rendered.
		if (!m_Mesh->GetVertexBufferRaw() || !m_Mesh->GetIndexBufferRaw())
			return;

		if (ShouldBeRendered())
			data.AddPrimitive(AsRenderProxy());
	}

	void MeshComponent::SetMeshFromAsset(const Asset& asset)
	{
		//m_MeshAsset = asset;
		//m_MeshResource = MeshResource::Query(asset);
		//m_MeshResource->Take([worldGuid = this->GetWorldContext()->GetGuid(), guid = this->GetGUID()](TShared<Mesh> mesh)
		//{
		//	World* world = g_Engine->FindWorld(worldGuid);
		//	Component* component = world->GetComponentRegistry().FindComponentByGUID(guid);
		//	ionassert(component->IsOfType<MeshComponent>());
		//	MeshComponent* meshComponent = (MeshComponent*)component;
		//	meshComponent->SetMesh(mesh);
		//});
	}

	//void MeshComponent::Tick(float deltaTime)
	//{
	//	Component::Tick(deltaTime);
	//	LOG_INFO("MeshComponent::Tick({0})", deltaTime);
	//}

	void MeshComponent::SetMesh(const TShared<Mesh>& mesh)
	{
		m_Mesh = mesh;
	}

	TShared<Mesh> MeshComponent::GetMesh() const
	{
		return m_Mesh;
	}

	RPrimitiveRenderProxy MeshComponent::AsRenderProxy() const
	{
		ionassert(m_Mesh);

		Material* material = m_Mesh->GetMaterial().get();
		Transform worldTransform = GetWorldTransform();

		RHIShader* shader = material ?
			material->GetShader().get() :
			Renderer::Get()->GetBasicShader().get();

		RPrimitiveRenderProxy mesh { };
		mesh.Transform     = worldTransform.GetMatrix();
		mesh.Material      = material;
		mesh.Shader        = shader;
		mesh.VertexBuffer  = m_Mesh->GetVertexBufferRaw();
		mesh.IndexBuffer   = m_Mesh->GetIndexBufferRaw();
		mesh.UniformBuffer = m_Mesh->GetUniformBufferRaw();

		return mesh;
	}
}
