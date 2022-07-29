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

		const MeshResourceDefaults& defaults = resource->GetDefaults();

		for (int32 i = 0; i < defaults.MaterialAssets.size(); ++i)
		{
			Asset asset = defaults.MaterialAssets[i];
			if (!asset)
				continue;

			TShared<MaterialInstance> material = MaterialRegistry::QueryMaterialInstance(asset);

			mesh->AssignMaterialToSlot(i, material);
		}

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
		// @TODO: This checking for compiled shit is really wrong, there should be a system that does this automatically
		if (instance && instance->GetBaseMaterial()->IsCompiled(EShaderUsage::StaticMesh))
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

		TShared<Material> baseMaterial = material->GetBaseMaterial();

		// @TODO: I don't think there should even be an option to assign not compiled materials to meshes.

		auto setLayoutShader = [this, material]
		{
			if (m_VertexBuffer)
			{
				m_VertexBuffer->SetLayoutShader(material->GetBaseMaterial()->GetShader(EShaderUsage::StaticMesh));
			}
		};

		// Compile the shaders first
		if (!baseMaterial->IsUsableWith(EShaderUsage::StaticMesh))
		{
			baseMaterial->AddUsage(EShaderUsage::StaticMesh);
			baseMaterial->CompileShaders([setLayoutShader](const ShaderPermutation& shader)
			{
				setLayoutShader();
			});
		}
		else
		{
			setLayoutShader();
		}

		MaterialSlot& slot = m_MaterialSlots.at(index);
		slot.MaterialInstance = material;
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
