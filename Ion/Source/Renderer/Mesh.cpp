#include "IonPCH.h"

#include "Mesh.h"
#include "RHI/UniformBuffer.h"
#include "Resource/TextureResource.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	TShared<Mesh> Mesh::Create()
	{
		return MakeShareable(new Mesh);
	}

	TShared<Mesh> Mesh::CreateFromResource(const TResourcePtr<MeshResource>& resource)
	{
		TShared<Mesh> mesh = Mesh::Create();

		resource->Take([mesh](const MeshResourceRenderDataShared& renderData)
		{
			mesh->SetVertexBuffer(renderData.VertexBuffer);
			mesh->SetIndexBuffer(renderData.IndexBuffer);
		});
		// @TODO: Is this fine?
		mesh->m_MeshResource = resource;

		return mesh;
	}

	Mesh::Mesh() :
		m_VertexBuffer(nullptr),
		m_IndexBuffer(nullptr),
		m_VertexCount(0),
		m_TriangleCount(0),
		m_UniformBuffer(MakeShareable(RHIUniformBuffer::Create<MeshUniforms>())),
		m_MaterialSlots(1, MaterialSlot())
	{
	}

	bool Mesh::LoadFromAsset(Asset& asset)
	{
		//ionassert(asset->GetType() == EAssetType::Mesh, "The asset is not a mesh.");

		//if (!asset.IsValid() || !asset->IsLoaded())
		//	return false;

		//const AssetDescription::Mesh* meshDesc = asset->GetDescription<EAssetType::Mesh>();
		//float* vertexAttributesPtr = (float*)((uint8*)asset->Data() + meshDesc->VerticesOffset);
		//uint32* indicesPtr = (uint32*)((uint8*)asset->Data() + meshDesc->IndicesOffset);

		//TShared<VertexBuffer> vb = VertexBuffer::Create(vertexAttributesPtr, meshDesc->VertexCount);
		//TShared<IndexBuffer> ib = IndexBuffer::Create(indicesPtr, (uint32)meshDesc->IndexCount);
		//vb->SetLayout(meshDesc->VertexLayout);

		//SetVertexBuffer(vb);
		//SetIndexBuffer(ib);

		return true;
	}

	void Mesh::SetVertexBuffer(const TShared<RHIVertexBuffer>& vertexBuffer)
	{
		m_VertexBuffer = vertexBuffer;
		m_VertexCount = vertexBuffer->GetVertexCount();

		// Set the layout if the material has been set before the VB.
		if (m_Material)
		{
			m_VertexBuffer->SetLayoutShader(m_Material->GetShader());
		}

		// @TODO: Handle each material slot in the future
		TShared<MaterialInstance> instance = m_MaterialSlots.at(0).MaterialInstance;
		if (instance)
		{
			m_VertexBuffer->SetLayoutShader(instance->GetBaseMaterial()->GetShader(EShaderUsage::StaticMesh));
		}
	}

	void Mesh::SetIndexBuffer(const TShared<RHIIndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
		m_TriangleCount = indexBuffer->GetTriangleCount();
	}

	void Mesh::SetMaterial(const TShared<MaterialOld>& material)
	{
		m_Material = material;

		// Vertex Buffer might not have been set yet.
		if (m_VertexBuffer)
		{
			m_VertexBuffer->SetLayoutShader(material->GetShader());
		}
	}

	const TShared<RHIVertexBuffer>& Mesh::GetVertexBuffer() const
	{
		return m_VertexBuffer;
	}

	const TShared<RHIIndexBuffer>& Mesh::GetIndexBuffer() const
	{
		return m_IndexBuffer;
	}

	const TShared<MaterialOld>& Mesh::GetMaterial() const
	{
		return m_Material;
	}

	void Mesh::AssignMaterialToSlot(uint16 index, const TShared<MaterialInstance>& material)
	{
		ionassert(index < m_MaterialSlots.size(), "Slot %i does not exist.", index);

		MaterialSlot& slot = m_MaterialSlots.at(index);
		slot.MaterialInstance = material;

		if (m_VertexBuffer)
		{
			m_VertexBuffer->SetLayoutShader(material->GetBaseMaterial()->GetShader(EShaderUsage::StaticMesh));
		}
	}

	TShared<MaterialInstance> Mesh::GetMaterialInSlot(uint16 index) const
	{
		ionassert(index < m_MaterialSlots.size(), "Slot %i does not exist.", index);

		const MaterialSlot& slot = m_MaterialSlots.at(index);
		return slot.MaterialInstance;
	}

	MeshUniforms& Mesh::GetUniformsDataRef()
	{
		return m_UniformBuffer->DataRef<MeshUniforms>();
	}

	const RHIVertexBuffer* Mesh::GetVertexBufferRaw() const
	{
		return m_VertexBuffer.get();
	}

	const RHIIndexBuffer* Mesh::GetIndexBufferRaw() const
	{
		return m_IndexBuffer.get();
	}

	const RHIUniformBuffer* Mesh::GetUniformBufferRaw() const
	{
		return m_UniformBuffer.get();
	}

	const MaterialOld* Mesh::GetMaterialRaw() const
	{
		return m_Material.get();
	}
}
