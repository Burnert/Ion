#include "IonPCH.h"

#include "MeshComponent.h"
#include "Engine/World.h"
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

		if (ShouldBeRendered())
			data.AddPrimitive(AsRenderProxy());
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

		Material* material = m_Mesh->GetMaterial().lock().get();
		Transform worldTransform = GetWorldTransform();

		RPrimitiveRenderProxy mesh { };
		mesh.Transform     = worldTransform.GetMatrix();
		mesh.Material      = material;
		mesh.Shader        = material->GetShader().get();
		mesh.VertexBuffer  = m_Mesh->GetVertexBufferRaw();
		mesh.IndexBuffer   = m_Mesh->GetIndexBufferRaw();
		mesh.UniformBuffer = m_Mesh->GetUniformBufferRaw();

		return mesh;
	}
}
