#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

namespace Ion
{
	class ION_API Mesh : public IDrawable
	{
	public:
		static TShared<Mesh> Create();

		virtual ~Mesh() { }

		void SetTransform(const Matrix4& transform);
		FORCEINLINE const Matrix4& GetTransform() const { return m_TransformMatrix; }

		void SetVertexBuffer(const TShared<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TShared<IndexBuffer>& indexBuffer);
		void SetMaterial(const TShared<Material>& material);

		const TShared<VertexBuffer>& GetVertexBuffer() const;
		const TShared<IndexBuffer>& GetIndexBuffer() const;
		const TShared<Material>& GetMaterial() const;

		// IDrawable:

		virtual const VertexBuffer* GetVertexBufferRaw() const override;
		virtual const IndexBuffer* GetIndexBufferRaw() const override;
		virtual const Material* GetMaterialRaw() const override;
		virtual const Matrix4& GetTransformMatrix() const override;

		// End of IDrawable

	protected:
		Mesh();

	private:
		TShared<VertexBuffer> m_VertexBuffer;
		TShared<IndexBuffer> m_IndexBuffer;
		TShared<Material> m_Material;

		uint32 m_VertexCount;
		uint32 m_TriangleCount;

		Matrix4 m_TransformMatrix;
	};
}
