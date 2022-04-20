#pragma once

#include "RendererCore.h"
#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "Material.h"
#include "Core/Asset/Asset.h"

namespace Ion
{
	struct UNIFORMBUFFER MeshUniforms
	{
		Matrix4 ModelViewProjectionMatrix;
		Matrix4 TransformMatrix;
		Matrix4 InverseTransposeMatrix;

		// With Editor

		UVector4 RenderGuid;
	};

	class UniformBuffer;

	class ION_API Mesh
	{
	public:
		static TShared<Mesh> Create();

		virtual ~Mesh() { }

		void SetVertexBuffer(const TShared<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TShared<IndexBuffer>& indexBuffer);
		void SetMaterial(const TShared<Material>& material);

		const TShared<VertexBuffer>& GetVertexBuffer() const;
		const TShared<IndexBuffer>& GetIndexBuffer() const;
		const TWeak<Material>& GetMaterial() const;

		MeshUniforms& GetUniformsDataRef();

		bool LoadFromAsset(Asset& asset);

		const VertexBuffer* GetVertexBufferRaw() const;
		const IndexBuffer* GetIndexBufferRaw() const;
		const UniformBuffer* GetUniformBufferRaw() const;
		const Material* GetMaterialRaw() const;

	private:
		Mesh();

	private:
		TShared<VertexBuffer> m_VertexBuffer;
		TShared<IndexBuffer> m_IndexBuffer;
		TShared<UniformBuffer> m_UniformBuffer;
		TWeak<Material> m_Material;

		uint32 m_VertexCount;
		uint32 m_TriangleCount;
	};
}
