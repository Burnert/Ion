#pragma once

#include "Core/Asset/Asset.h"

#include "Resource/MeshResource.h"

#include "RendererCore.h"
#include "Material.h"

#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"

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

	class ION_API Mesh
	{
	public:
		static TShared<Mesh> Create();
		static TShared<Mesh> CreateFromResource(const TShared<MeshResource>& resource);

		virtual ~Mesh() { }

		void SetVertexBuffer(const TShared<RHIVertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TShared<RHIIndexBuffer>& indexBuffer);
		void SetMaterial(const TShared<Material>& material);

		const TShared<RHIVertexBuffer>& GetVertexBuffer() const;
		const TShared<RHIIndexBuffer>& GetIndexBuffer() const;
		const TWeak<Material>& GetMaterial() const;

		MeshUniforms& GetUniformsDataRef();

		bool LoadFromAsset(Asset& asset);

		const RHIVertexBuffer* GetVertexBufferRaw() const;
		const RHIIndexBuffer* GetIndexBufferRaw() const;
		const RHIUniformBuffer* GetUniformBufferRaw() const;
		const Material* GetMaterialRaw() const;

	private:
		Mesh();

	private:
		TShared<RHIVertexBuffer> m_VertexBuffer;
		TShared<RHIIndexBuffer> m_IndexBuffer;
		TShared<RHIUniformBuffer> m_UniformBuffer;
		TWeak<Material> m_Material;

		uint32 m_VertexCount;
		uint32 m_TriangleCount;
	};
}
