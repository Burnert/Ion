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

		TShared<Material> material = Material::Create();
		material->SetShader(Renderer::GetBasicShader());
		material->CreateParameter("Texture", EMaterialParameterType::Texture2D);

		mesh->SetMaterial(material);

		// Set the defaults
		const MeshResourceDefaults& defaults = resource->GetDefaults();
		// All assets should have been loaded by now.
		if (defaults.TextureAsset)
		{
			// @TODO: Do something so it makes more sense when the material system is done

			// @TODO: Temporary -> the material system will load and instantiate the textures
			mesh->m_Texture = TextureResource::Query(defaults.TextureAsset);

			mesh->m_Texture->Take([material](const TextureResourceRenderDataShared& data)
			{
				material->SetParameter("Texture", data.Texture);
			});
		}
		else
		{
			material->SetParameter("Texture", Renderer::GetWhiteTexture());
		}

		return mesh;
	}

	Mesh::Mesh() :
		m_VertexBuffer(nullptr),
		m_IndexBuffer(nullptr),
		m_VertexCount(0),
		m_TriangleCount(0),
		m_UniformBuffer(MakeShareable(RHIUniformBuffer::Create<MeshUniforms>()))
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
	}

	void Mesh::SetIndexBuffer(const TShared<RHIIndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
		m_TriangleCount = indexBuffer->GetTriangleCount();
	}

	void Mesh::SetMaterial(const TShared<Material>& material)
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

	const TShared<Material>& Mesh::GetMaterial() const
	{
		return m_Material;
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

	const Material* Mesh::GetMaterialRaw() const
	{
		return m_Material.get();
	}
}
