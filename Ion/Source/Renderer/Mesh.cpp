#include "IonPCH.h"

#include "Mesh.h"

namespace Ion
{
	TShared<Mesh> Mesh::Create()
	{
		return MakeShareable(new Mesh);
	}

	Mesh::Mesh() :
		m_VertexCount(0),
		m_TriangleCount(0),
		m_TransformMatrix(Matrix4(1.0f))
	{ }

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
	}

	const TShared<VertexBuffer>& Mesh::GetVertexBuffer() const
	{
		return m_VertexBuffer;
	}

	const TShared<IndexBuffer>& Mesh::GetIndexBuffer() const
	{
		return m_IndexBuffer;
	}

	const TShared<Material>& Mesh::GetMaterial() const
	{
		return m_Material;
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

	const Material* Mesh::GetMaterialRaw() const
	{
		return m_Material.get();
	}

	const Matrix4& Mesh::GetTransformMatrix() const
	{
		return m_TransformMatrix;
	}

	// End of IDrawable
}
