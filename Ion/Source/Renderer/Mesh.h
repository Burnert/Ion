#pragma once

#include "Core/Asset/Asset.h"

#include "Resource/MeshResource.h"
#include "Resource/TextureResource.h"

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
		static TShared<Mesh> CreateFromResource(const TResourcePtr<MeshResource>& resource);

		virtual ~Mesh() { }

		void SetVertexBuffer(RHIVertexBuffer* vertexBuffer);
		void SetIndexBuffer(RHIIndexBuffer* indexBuffer);
		void SetMaterial(const TShared<Material>& material);

		RHIVertexBuffer* GetVertexBuffer() const;
		RHIIndexBuffer* GetIndexBuffer() const;
		const TShared<Material>& GetMaterial() const;

		MeshUniforms& GetUniformsDataRef();

		bool LoadFromAsset(Asset& asset);

		const RHIVertexBuffer* GetVertexBufferRaw() const;
		const RHIIndexBuffer* GetIndexBufferRaw() const;
		const RHIUniformBuffer* GetUniformBufferRaw() const;
		const Material* GetMaterialRaw() const;

	private:
		Mesh();

	private:
		RHIVertexBuffer* m_VertexBuffer;
		RHIIndexBuffer* m_IndexBuffer;
		TShared<RHIUniformBuffer> m_UniformBuffer;
		TShared<Material> m_Material;

		TResourcePtr<TextureResource> m_Texture;

		uint32 m_VertexCount;
		uint32 m_TriangleCount;
	};
}
