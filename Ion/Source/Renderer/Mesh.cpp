#include "IonPCH.h"

#include "Mesh.h"
#include "UniformBuffer.h"

namespace Ion
{
	TShared<Mesh> Mesh::Create()
	{
		return MakeShareable(new Mesh);
	}

	TShared<Mesh> Mesh::Create(AssetHandle& asset)
	{
		return MakeShareable(new Mesh(asset));
	}

	Mesh::Mesh() :
		m_VertexCount(0),
		m_TriangleCount(0),
		m_TransformMatrix(Matrix4(1.0f)),
		m_UniformBuffer(MakeShareable(UniformBuffer::Create<MeshUniforms>()))
	{
	}

	Mesh::Mesh(AssetHandle& asset) :
		Mesh()
	{
		LoadFromAsset(asset);
	}

	bool Mesh::LoadFromAsset(AssetHandle& asset)
	{
		if (!asset.IsValid() || !asset->IsLoaded())
			return false;

		m_MeshAsset = asset;

		const AssetDescription::Mesh* meshDesc = m_MeshAsset->GetDescription<EAssetType::Mesh>();
		float* vertexAttributesPtr = (float*)((uint8*)m_MeshAsset->Data() + meshDesc->VerticesOffset);
		uint32* indicesPtr = (uint32*)((uint8*)m_MeshAsset->Data() + meshDesc->IndicesOffset);

		SetVertexBuffer(VertexBuffer::Create(vertexAttributesPtr, meshDesc->VertexCount));
		SetIndexBuffer(IndexBuffer::Create(indicesPtr, (uint32)meshDesc->IndexCount));

		m_VertexBuffer->SetLayout(meshDesc->VertexLayout);

		return true;
	}

	void Mesh::SetTransform(const Matrix4& transform)
	{
		m_TransformMatrix = transform;
	}

	void Mesh::SetVertexBuffer(const TShared<VertexBuffer>& vertexBuffer)
	{
		m_VertexBuffer = vertexBuffer;
		m_VertexCount = vertexBuffer->GetVertexCount();
	}

	void Mesh::SetIndexBuffer(const TShared<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
		m_TriangleCount = indexBuffer->GetTriangleCount();
	}

	void Mesh::SetMaterial(const TShared<Material>& material)
	{
		m_Material = material;
		m_VertexBuffer->SetLayoutShader(material->GetShader());
	}

	const TShared<VertexBuffer>& Mesh::GetVertexBuffer() const
	{
		return m_VertexBuffer;
	}

	const TShared<IndexBuffer>& Mesh::GetIndexBuffer() const
	{
		return m_IndexBuffer;
	}

	const TWeak<Material>& Mesh::GetMaterial() const
	{
		return m_Material;
	}

	MeshUniforms& Mesh::GetUniformsDataRef()
	{
		return m_UniformBuffer->DataRef<MeshUniforms>();
	}

	// IDrawable:

	const VertexBuffer* Mesh::GetVertexBufferRaw() const
	{
		return m_VertexBuffer.get();
	}

	const IndexBuffer* Mesh::GetIndexBufferRaw() const
	{
		return m_IndexBuffer.get();
	}

	const UniformBuffer* Mesh::GetUniformBufferRaw() const
	{
		return m_UniformBuffer.get();
	}

	const Material* Mesh::GetMaterialRaw() const
	{
		return m_Material.lock().get();
	}

	const Matrix4& Mesh::GetTransformMatrix() const
	{
		return m_TransformMatrix;
	}

	// End of IDrawable
}
