#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

namespace Ion
{
	class ION_API Mesh : public IDrawable
	{
	public:
		static TShared<Mesh> Create();

		virtual ~Mesh() { };

		void SetTransform(const FMatrix4& transform);
		FORCEINLINE const FMatrix4& GetTransform() const { return m_TransformMatrix; }

		void SetVertexBuffer(const TShared<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TShared<IndexBuffer>& indexBuffer);
		void SetShader(const TShared<Shader>& shader);

		// IDrawable:

		virtual TShared<VertexBuffer> GetVertexBuffer() const override;
		virtual TShared<IndexBuffer> GetIndexBuffer() const override;
		virtual TShared<Shader> GetShader() const override;

		// End of IDrawable

	protected:
		Mesh();

	private:
		TShared<VertexBuffer> m_VertexBuffer;
		TShared<IndexBuffer> m_IndexBuffer;
		TShared<Shader> m_Shader;

		uint m_VertexCount;
		uint m_TriangleCount;

		FMatrix4 m_TransformMatrix;
	};
}
