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
	};

	/**
	 * @brief Resource description representation from the asset file.
	 */
	struct MeshResourceDescription
	{
		MeshResourceDefaults Defaults;
		int8 Unused;
	};

	struct MeshResourceRenderData
	{
		// @TODO: these shouldn't be shader ptrs

		TShared<RHIVertexBuffer> VertexBuffer;
		TShared<RHIIndexBuffer> IndexBuffer;

		bool IsAvailable() const
		{
			return VertexBuffer && IndexBuffer;
		}
	};

	class ION_API MeshResource : public Resource
	{
	public:
		using TResourceDescription = MeshResourceDescription;

		static TShared<MeshResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the mesh render data owned by the Resource.
		 * 
		 * @tparam Lambda params - (const MeshResourceRenderData&)
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
		 * @param path .iasset file path
		 * @param outGuid GUID object to write the resource Guid to.
		 * @param outDescription MeshResourceDescription object to write to
		 * @return True if the file has been parsed successfully.
		 */
		static bool ParseAssetFile(const FilePath& path, GUID& outGuid, MeshResourceDescription& outDescription);

	protected:
		MeshResource(const GUID& guid, const Asset& asset, const MeshResourceDescription& desc) :
			Resource(guid, asset),
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
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<MeshResourceRenderData>>);

		if (m_RenderData.IsAvailable())
		{
			onTake(m_RenderData);
			return true;
		}

		auto initMesh = [this, onTake](const AssetData& data)
		{
			ionassert(m_Asset->GetType() == EAssetType::Mesh);

			TShared<MeshAssetData> meshData = data.Get<MeshAssetData>();

			m_RenderData.VertexBuffer = RHIVertexBuffer::Create(meshData->Vertices.Ptr, meshData->Vertices.Count);
			m_RenderData.VertexBuffer->SetLayout(meshData->Layout);

			m_RenderData.IndexBuffer = RHIIndexBuffer::Create(meshData->Indices.Ptr, (uint32)meshData->Indices.Count);

			onTake(m_RenderData);
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
