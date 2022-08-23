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
		std::shared_ptr<RHIVertexBuffer> VertexBuffer;
		std::shared_ptr<RHIIndexBuffer> IndexBuffer;
	};

	struct MeshResourceRenderData
	{
		std::weak_ptr<RHIVertexBuffer> VertexBuffer;
		std::weak_ptr<RHIIndexBuffer> IndexBuffer;

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

		static TResourceRef<MeshResource> Query(const Asset& asset);

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

		DEFINE_RESOURCE_AS_REF(MeshResource)

	protected:
		MeshResource(const Asset& asset, const MeshResourceDescription& desc) :
			Resource(asset),
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

		ionassert(m_Asset);

		if (m_RenderData.IsAvailable())
		{
			onTake(m_RenderData.Lock());
			return true;
		}

		ResourceLogger.Trace("Importing Mesh Resource from Asset \"{}\".", m_Asset->GetVirtualPath());
		m_Asset->Import(
			[](std::shared_ptr<AssetFileMemoryBlock> block)
			{
				return AssetImporter::ImportColladaMeshAsset(block);
			},
			// Store the ref (self) so the resource doesn't get deleted before it's loaded
			[this, self = AsRef(), onTake](std::shared_ptr<MeshAssetData> meshData)
			{
				ionassert(m_Asset);
				ionassert(m_Asset->GetType() == EAssetType::Mesh);

				MeshResourceRenderDataShared sharedRenderData { };

				// The same manual reference counting as in TextureResource.

				// RHIVertexBuffer:
				//ResourceMemory::IncRef(*this);
				RHIVertexBuffer* vb = RHIVertexBuffer::Create(meshData->Vertices.Ptr, meshData->Vertices.Count);
				sharedRenderData.VertexBuffer = std::shared_ptr<RHIVertexBuffer>(vb, [this](RHIVertexBuffer* ptr)
				{
					//ResourceMemory::DecRef(*this);
					delete ptr;
				});

				// RHIIndexBuffer:
				//ResourceMemory::IncRef(*this);
				RHIIndexBuffer* ib = RHIIndexBuffer::Create(meshData->Indices.Ptr, (uint32)meshData->Indices.Count);
				sharedRenderData.IndexBuffer = std::shared_ptr<RHIIndexBuffer>(ib, [this](RHIIndexBuffer* ptr)
				{
					//ResourceMemory::DecRef(*this);
					delete ptr;
				});

				sharedRenderData.VertexBuffer->SetLayout(meshData->Layout);

				onTake(sharedRenderData);

				m_RenderData = sharedRenderData;

				ResourceLogger.Trace("Imported Mesh Resource from Asset \"{}\" successfully.", m_Asset->GetVirtualPath());
			},
			[&](auto& result) { ResourceLogger.Error("Failed to import Mesh Resource from Asset \"{}\". {}", m_Asset->GetVirtualPath(), result.GetErrorMessage()); }
		);
		return false;
	}

	inline const MeshResourceDefaults& MeshResource::GetDefaults() const
	{
		return m_Description.Defaults;
	}
}
