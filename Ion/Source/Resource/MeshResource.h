#pragma once

#include "Resource.h"
#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Mesh Resource
	// ------------------------------------------------------------

	struct MeshResourceDefaults
	{
		Asset TextureAsset;
		TArray<Asset> MaterialAssets;
	};

	/**
	 * @brief Resource description representation from the asset file.
	 */
	struct MeshResourceDescription
	{
		MeshResourceDefaults Defaults;
	};

	struct MeshResourceRenderDataShared
	{
		TShared<RHIVertexBuffer> VertexBuffer;
		TShared<RHIIndexBuffer> IndexBuffer;
	};

	struct MeshResourceRenderData
	{
		TWeak<RHIVertexBuffer> VertexBuffer;
		TWeak<RHIIndexBuffer> IndexBuffer;

		bool IsAvailable() const
		{
			return !VertexBuffer.expired() && !IndexBuffer.expired();
		}

		MeshResourceRenderDataShared Lock() const
		{
			MeshResourceRenderDataShared data { };
			if (IsAvailable())
			{
				data.VertexBuffer = VertexBuffer.lock();
				data.IndexBuffer = IndexBuffer.lock();
			}
			return data;
		}

		MeshResourceRenderData& operator=(const MeshResourceRenderDataShared& shared)
		{
			VertexBuffer = shared.VertexBuffer;
			IndexBuffer = shared.IndexBuffer;
			return *this;
		}
	};

	class ION_API MeshResource : public Resource
	{
	public:
		using TResourceDescription = MeshResourceDescription;

		static TResourcePtr<MeshResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the mesh render data owned by the Resource.
		 * 
		 * @tparam Lambda params - (const MeshResourceRenderDataShared&)
		 * @see TFuncResourceOnTake
		 * 
		 * @param onTake If the resource is ready, called immediately,
		 * else, called as soon as the resource is loaded.
		 * 
		 * @return Returns true if the resource is available instantly.
		 */
		template<typename Lambda>
		bool Take(Lambda onTake);

		const MeshResourceDefaults& GetDefaults() const;

		virtual bool IsLoaded() const override;

		/**
		 * @brief Parses the MeshResource node in the .iasset file.
		 * Called by Resource::Query
		 *
		 * @see Resource::Query
		 *
		 * @param asset Asset handle
		 * @param outGuid GUID object to write the resource Guid to.
		 * @param outDescription MeshResourceDescription object to write to
		 * @return True if the file has been parsed successfully.
		 */
		static bool ParseAssetFile(const Asset& asset, GUID& outGuid, MeshResourceDescription& outDescription);

	protected:
		MeshResource(const GUID& guid, const Asset& asset, const MeshResourceDescription& desc) :
			Resource(guid, asset),
			m_RenderData({ }),
			m_Description(desc)
		{
		}

	private:
		MeshResourceRenderData m_RenderData;
		MeshResourceDescription m_Description;

		friend class Resource;
	};

	template<typename Lambda>
	inline bool MeshResource::Take(Lambda onTake)
	{
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<MeshResourceRenderDataShared>>);

		if (m_RenderData.IsAvailable())
		{
			onTake(m_RenderData.Lock());
			return true;
		}

		auto initMesh = [this, onTake](const AssetData& data)
		{
			ionassert(m_Asset->GetType() == EAssetType::Mesh);

			TShared<MeshAssetData> meshData = data.Get<MeshAssetData>();

			MeshResourceRenderDataShared sharedRenderData { };

			// The same manual reference counting as in TextureResource.

			// RHIVertexBuffer:
			ResourceMemory::IncRef(*this);
			RHIVertexBuffer* vb = RHIVertexBuffer::Create(meshData->Vertices.Ptr, meshData->Vertices.Count);
			sharedRenderData.VertexBuffer = TShared<RHIVertexBuffer>(vb, [this](RHIVertexBuffer* ptr)
			{
				ResourceMemory::DecRef(*this);
				delete ptr;
			});

			// RHIIndexBuffer:
			ResourceMemory::IncRef(*this);
			RHIIndexBuffer* ib = RHIIndexBuffer::Create(meshData->Indices.Ptr, (uint32)meshData->Indices.Count);
			sharedRenderData.IndexBuffer = TShared<RHIIndexBuffer>(ib, [this](RHIIndexBuffer* ptr)
			{
				ResourceMemory::DecRef(*this);
				delete ptr;
			});

			sharedRenderData.VertexBuffer->SetLayout(meshData->Layout);

			onTake(sharedRenderData);

			m_RenderData = sharedRenderData;
		};

		TOptional<AssetData> data = m_Asset->Load(initMesh);

		if (data)
		{
			initMesh(data.value());
			return true;
		}

		return false;
	}

	inline const MeshResourceDefaults& MeshResource::GetDefaults() const
	{
		return m_Description.Defaults;
	}
}
