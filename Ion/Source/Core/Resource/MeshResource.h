#pragma once

#include "Resource.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Mesh Resource
	// ------------------------------------------------------------

	class ION_API MeshResource : public Resource
	{
	public:
		static TShared<MeshResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the Mesh owned by the Resource.
		 * 
		 * @tparam Lambda Must be convertible to TFuncResourceOnTake<Mesh>
		 * @see TFuncResourceOnTake
		 * 
		 * @param onTake If the resource is ready, called immediately,
		 * else, called when the asset and the resource get loaded.
		 * 
		 * @return Returns true if the resource is available instantly.
		 */
		template<typename Lambda>
		bool Take(Lambda onTake);

	protected:
		MeshResource(const Asset& asset) :
			Resource(asset)
		{
		}

	private:
		TShared<Mesh> m_Mesh;

		friend class Resource;
	};

	template<typename Lambda>
	inline bool MeshResource::Take(Lambda onTake)
	{
		if (m_Mesh)
		{
			onTake(m_Mesh);
			return true;
		}

		auto initMesh = [this, onTake](const AssetData& data)
		{
			ionassert(m_Asset->GetType() == EAssetType::Mesh);

			TShared<MeshAssetData> mesh = data.Get<MeshAssetData>();

			m_Mesh = Mesh::Create();

			TShared<RHIVertexBuffer> vb = RHIVertexBuffer::Create(mesh->Vertices.Ptr, mesh->Vertices.Count);
			TShared<RHIIndexBuffer> ib = RHIIndexBuffer::Create(mesh->Indices.Ptr, (uint32)mesh->Indices.Count);
			vb->SetLayout(mesh->Layout);

			m_Mesh->SetVertexBuffer(vb);
			m_Mesh->SetIndexBuffer(ib);

			onTake(m_Mesh);
		};

		TOptional<AssetData> data = m_Asset->Load(initMesh);

		if (data)
		{
			initMesh(data.value());
			return true;
		}

		return false;
	}
}
