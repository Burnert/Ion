#pragma once

#include "RendererCore.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

namespace Ion
{
	struct UNIFORMBUFFER MeshUniforms
	{
		Matrix4 ModelViewProjectionMatrix;
		Matrix4 TransformMatrix;
		Matrix4 InverseTransposeMatrix;
	};

	class UniformBuffer;

	class ION_API Mesh : public IDrawable
	{
	public:
		static TShared<Mesh> Create();
		static TShared<Mesh> Create(AssetHandle& asset);

		virtual ~Mesh() { }

		void SetTransform(const Matrix4& transform);
		FORCEINLINE const Matrix4& GetTransform() const { return m_TransformMatrix; }

		void SetVertexBuffer(const TShared<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TShared<IndexBuffer>& indexBuffer);
		void SetMaterial(const TShared<Material>& material);

		const TShared<VertexBuffer>& GetVertexBuffer() const;
		const TShared<IndexBuffer>& GetIndexBuffer() const;
		const TWeak<Material>& GetMaterial() const;

		MeshUniforms& GetUniformsDataRef();

		// IDrawable:

		virtual const VertexBuffer* GetVertexBufferRaw() const override;
		virtual const IndexBuffer* GetIndexBufferRaw() const override;
		virtual const UniformBuffer* GetUniformBufferRaw() const override;
		virtual const Material* GetMaterialRaw() const override;
		virtual const Matrix4& GetTransformMatrix() const override;

		// End of IDrawable

	private:
		Mesh();
		Mesh(AssetHandle& asset);

		bool LoadFromAsset(AssetHandle& asset);

	private:
		AssetHandle m_MeshAsset;

		TShared<VertexBuffer> m_VertexBuffer;
		TShared<IndexBuffer> m_IndexBuffer;
		TShared<UniformBuffer> m_UniformBuffer;
		TWeak<Material> m_Material;

		uint32 m_VertexCount;
		uint32 m_TriangleCount;

		Matrix4 m_TransformMatrix;
	};
}
