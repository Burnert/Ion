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

		const TShared<VertexBuffer>& GetVertexBuffer() const;
		const TShared<IndexBuffer>& GetIndexBuffer() const;
		const TShared<Material>& GetMaterial() const;

		// IDrawable:

		virtual const VertexBuffer* GetVertexBufferRaw() const override;
		virtual const IndexBuffer* GetIndexBufferRaw() const override;
		virtual const Material* GetMaterialRaw() const override;
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
