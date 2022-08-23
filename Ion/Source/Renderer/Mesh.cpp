#include "IonPCH.h"

#include "Mesh.h"
#include "RHI/UniformBuffer.h"
#include "Resource/TextureResource.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	std::shared_ptr<Mesh> Mesh::Create()
	{
		return MakeShareable(new Mesh);
	}

	std::shared_ptr<Mesh> Mesh::CreateFromResource(const TResourceRef<MeshResource>& resource)
	{
		std::shared_ptr<Mesh> mesh = Mesh::Create();

		resource->Take([mesh](const MeshResourceRenderDataShared& renderData)
		{
			mesh->SetVertexBuffer(renderData.VertexBuffer);
			mesh->SetIndexBuffer(renderData.IndexBuffer);
		});
		// Reference the resource, so it doesn't get deleted until the mesh is alive.
		mesh->m_MeshResource = resource;

		const MeshResourceDefaults& defaults = resource->GetDefaults();

		for (int32 i = 0; i < defaults.MaterialAssets.size(); ++i)
		{
			Asset asset = defaults.MaterialAssets[i];
			if (!asset)
				continue;

			std::shared_ptr<MaterialInstance> material = MaterialRegistry::QueryMaterialInstance(asset);

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

		//std::shared_ptr<VertexBuffer> vb = VertexBuffer::Create(vertexAttributesPtr, meshDesc->VertexCount);
		//std::shared_ptr<IndexBuffer> ib = IndexBuffer::Create(indicesPtr, (uint32)meshDesc->IndexCount);
		//vb->SetLayout(meshDesc->VertexLayout);

		//SetVertexBuffer(vb);
		//SetIndexBuffer(ib);

		return true;
	}

	void Mesh::SetVertexBuffer(const std::shared_ptr<RHIVertexBuffer>& vertexBuffer)
	{
		m_VertexBuffer = vertexBuffer;
		m_VertexCount = vertexBuffer->GetVertexCount();

		// Set the layout if the material has been set before the VB.

		// @TODO: Handle each material slot in the future
		std::shared_ptr<MaterialInstance> instance = m_MaterialSlots.at(0).MaterialInstance;
		// @TODO: This checking for compiled shit is really wrong, there should be a system that does this automatically
		if (instance && instance->GetBaseMaterial()->IsCompiled(EShaderUsage::StaticMesh))
		{
			m_VertexBuffer->SetLayoutShader(instance->GetBaseMaterial()->GetShader(EShaderUsage::StaticMesh));
		}
	}

	void Mesh::SetIndexBuffer(const std::shared_ptr<RHIIndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
		m_TriangleCount = indexBuffer->GetTriangleCount();
	}

	const std::shared_ptr<RHIVertexBuffer>& Mesh::GetVertexBuffer() const
	{
		return m_VertexBuffer;
	}

	const std::shared_ptr<RHIIndexBuffer>& Mesh::GetIndexBuffer() const
	{
		return m_IndexBuffer;
	}

	void Mesh::AssignMaterialToSlot(uint16 index, const std::shared_ptr<MaterialInstance>& material)
	{
		ionassert(index < m_MaterialSlots.size(), "Slot {0} does not exist.", index);

		if (material)
		{
			std::shared_ptr<Material> baseMaterial = material->GetBaseMaterial();

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
		}

		MaterialSlot& slot = m_MaterialSlots.at(index);
		slot.MaterialInstance = material;
	}

	std::shared_ptr<MaterialInstance> Mesh::GetMaterialInSlot(uint16 index) const
	{
		ionassert(index < m_MaterialSlots.size(), "Slot {0} does not exist.", index);

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
}
