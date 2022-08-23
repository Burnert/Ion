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

	void MeshComponent::OnCreate()
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

	void MeshComponent::SetMeshAsset(const Asset& asset)
	{
		m_MeshAsset = asset;
	}

	void MeshComponent::SetMeshResource(const TResourceRef<MeshResource>& resource)
	{
		m_MeshResource = resource;
	}

	void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& mesh)
	{
		m_Mesh = mesh;
	}

	std::shared_ptr<Mesh> MeshComponent::GetMesh() const
	{
		return m_Mesh;
	}

	RPrimitiveRenderProxy MeshComponent::AsRenderProxy() const
	{
		ionassert(m_Mesh);

		Transform worldTransform = GetWorldTransform();

		RHIShader* shader = Renderer::Get()->GetBasicShader().get();

		std::shared_ptr<MaterialInstance> materialInstance = m_Mesh->GetMaterialInSlot(0);
		if (materialInstance)
		{
			const std::shared_ptr<Material>& material = materialInstance->GetBaseMaterial();
			if (material)
			{
				shader = material->GetShader(EShaderUsage::StaticMesh).get();
			}
		}

		RPrimitiveRenderProxy mesh { };
		mesh.Transform        = worldTransform.GetMatrix();
		mesh.MaterialInstance = materialInstance.get();
		mesh.Shader           = shader;
		mesh.VertexBuffer     = m_Mesh->GetVertexBufferRaw();
		mesh.IndexBuffer      = m_Mesh->GetIndexBufferRaw();
		mesh.UniformBuffer    = m_Mesh->GetUniformBufferRaw();

		return mesh;
	}
}
