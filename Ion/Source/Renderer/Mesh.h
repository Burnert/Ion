#pragma once

#include "Asset/Asset.h"

#include "Resource/MeshResource.h"
#include "Resource/TextureResource.h"

#include "RendererCore.h"

#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"

#include "Material/MaterialInstance.h"
#include "Material/MaterialRegistry.h"

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

	struct MaterialSlot
	{
		std::shared_ptr<MaterialInstance> MaterialInstance;
		uint16 Index;
		union
		{
			uint16 Flags;
			struct
			{
				uint16 bCastShadows : 1;
			};
		};
	};

	class ION_API Mesh
	{
	public:
		static std::shared_ptr<Mesh> Create();
		static std::shared_ptr<Mesh> CreateFromResource(const TResourceRef<MeshResource>& resource);

		virtual ~Mesh() { }

		void SetVertexBuffer(const TRef<RHIVertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const TRef<RHIIndexBuffer>& indexBuffer);

		const TRef<RHIVertexBuffer>& GetVertexBuffer() const;
		const TRef<RHIIndexBuffer>& GetIndexBuffer() const;

		void AssignMaterialToSlot(uint16 index, const std::shared_ptr<MaterialInstance>& material);
		std::shared_ptr<MaterialInstance> GetMaterialInSlot(uint16 slot) const;

		MeshUniforms& GetUniformsData();

		bool LoadFromAsset(Asset& asset);

		const RHIVertexBuffer* GetVertexBufferRaw() const;
		const RHIIndexBuffer* GetIndexBufferRaw() const;
		const RHIUniformBuffer* GetUniformBufferRaw() const;

		const TResourceRef<MeshResource>& GetMeshResource() const;

	private:
		Mesh();

	private:
		TArray<MaterialSlot> m_MaterialSlots;

		TRef<RHIVertexBuffer> m_VertexBuffer;
		TRef<RHIIndexBuffer> m_IndexBuffer;
		TRef<RHIUniformBuffer> m_UniformBuffer;

		TResourceRef<MeshResource> m_MeshResource;

		TResourceRef<TextureResource> m_Texture;

		uint32 m_VertexCount;
		uint32 m_TriangleCount;
	};

	inline const TResourceRef<MeshResource>& Mesh::GetMeshResource() const
	{
		return m_MeshResource;
	}
}
