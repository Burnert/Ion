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

		void SetTransform(const FMatrix4& transform);
		FORCEINLINE const FMatrix4& GetTransform() const { return m_TransformMatrix; }

		void SetVertexBuffer(const TShared<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TShared<IndexBuffer>& indexBuffer);
		void SetMaterial(const TShared<Material>& material);

		// IDrawable:

		virtual const TShared<VertexBuffer>& GetVertexBuffer() const override;
		virtual const TShared<IndexBuffer>& GetIndexBuffer() const override;
		virtual const TShared<Material>& GetMaterial() const override;
		virtual const FMatrix4& GetTransformMatrix() const override;

		// End of IDrawable

	protected:
		Mesh();

	private:
		TShared<VertexBuffer> m_VertexBuffer;
		TShared<IndexBuffer> m_IndexBuffer;
		TShared<Material> m_Material;

		uint m_VertexCount;
		uint m_TriangleCount;

		FMatrix4 m_TransformMatrix;
	};
}
