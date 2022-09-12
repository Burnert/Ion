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

	struct MeshResourceRenderData
	{
		TRef<RHIVertexBuffer> VertexBuffer;
		TRef<RHIIndexBuffer> IndexBuffer;

		bool IsAvailable() const
		{
			return VertexBuffer && IndexBuffer;
		}
	};

#pragma region Mesh Asset Type

	class MeshAssetType : public IAssetType
	{
	public:
		virtual Result<TSharedPtr<IAssetCustomData>, IOError> Parse(const std::shared_ptr<XMLDocument>& xml) const override;
		virtual Result<void, IOError> Serialize(Archive& ar, TSharedPtr<IAssetCustomData>& inOutCustomData) const override;
		virtual TSharedPtr<IAssetCustomData> CreateDefaultCustomData() const override;
		ASSET_TYPE_NAME_IMPL("Ion.Mesh")
	};

	REGISTER_ASSET_TYPE_CLASS(MeshAssetType);

	class MeshAssetData : public IAssetCustomData
	{
	public:
		ASSET_DATA_GETTYPE_IMPL(AT_MeshAssetType)

		GUID ResourceGuid;
		MeshResourceDescription Description;
	};

	ASSET_TYPE_DEFAULT_DATA_INL_IMPL(MeshAssetType, MeshAssetData)

#pragma endregion

	class ION_API MeshResource : public Resource
	{
	public:
		using TResourceDescription = MeshResourceDescription;

		static TSharedPtr<MeshResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the mesh render data owned by the Resource.
		 * 
		 * @details Use the GetRenderData function to retrieve the loaded objects.
		 * 
		 * @tparam Lambda params - (const TSharedPtr<MeshResource>&)
		 * @see TFuncResourceOnTake
		 * 
		 * @param onTake If the resource is ready, called immediately,
		 * else, called as soon as the resource is loaded (on the main thread).
		 * 
		 * @return Returns true if the resource is available instantly.
		 */
		template<typename Lambda>
		bool Take(Lambda onTake);

		/**
		 * @brief Get the resource render data, even if it hasn't been loaded yet.
		 * 
		 * @return TextureResource render data
		 */
		const MeshResourceRenderData& GetRenderData() const;

		const MeshResourceDefaults& GetDefaults() const;

		virtual bool IsLoaded() const override;

	protected:
		MeshResource(const Asset& asset) :
			Resource(asset),
			m_RenderData({ })
		{
			ionassert(asset->GetType() == AT_MeshAssetType);
			TSharedPtr<MeshAssetData> data = PtrCast<MeshAssetData>(asset->GetCustomData());
			m_Description = data->Description;
		}

	private:
		MeshResourceRenderData m_RenderData;
		MeshResourceDescription m_Description;

		friend class Resource;
		FRIEND_MAKE_SHARED;
	};

	template<typename Lambda>
	inline bool MeshResource::Take(Lambda onTake)
	{
		static_assert(TIsConvertibleV<Lambda, TFuncResourceOnTake<MeshResource>>);

		ionassert(m_Asset);

		TSharedPtr<MeshResource> self = PtrCast<MeshResource>(SharedFromThis());

		if (m_RenderData.IsAvailable())
		{
			onTake(self);
			return true;
		}

		ResourceLogger.Trace("Mesh Resource \"{}\" render data is unavailable.", m_Asset->GetVirtualPath());
		m_Asset->Import(
			[self](std::shared_ptr<AssetFileMemoryBlock> block)
			{
				ResourceLogger.Trace("Importing Mesh Resource from Asset \"{}\"...", self->m_Asset->GetVirtualPath());
				return AssetImporter::ImportColladaMeshAsset(block);
			},
			// Store the ref (self) so the resource doesn't get deleted before it's loaded
			[this, self, onTake](std::shared_ptr<ImportedMeshData> meshData)
			{
				ResourceLogger.Info("Mesh Resource from Asset \"{}\" has been imported successfully.", m_Asset->GetVirtualPath());

				ionassert(m_Asset);
				ionassert(m_Asset->GetType() == AT_MeshAssetType);

				m_RenderData.VertexBuffer = RHIVertexBuffer::Create(meshData->Vertices.Ptr, meshData->Vertices.Count);
				m_RenderData.IndexBuffer = RHIIndexBuffer::Create(meshData->Indices.Ptr, (uint32)meshData->Indices.Count);

				m_RenderData.VertexBuffer->SetLayout(meshData->Layout);

				ResourceLogger.Trace("Mesh Resource \"{}\" render data is now available.", m_Asset->GetVirtualPath());

				onTake(self);
			},
			[self](auto& result) { ResourceLogger.Error("Failed to import Mesh Resource from Asset \"{}\". {}", self->m_Asset->GetVirtualPath(), result.GetErrorMessage()); }
		);
		return false;
	}

	inline const MeshResourceRenderData& MeshResource::GetRenderData() const
	{
		return m_RenderData;
	}

	inline const MeshResourceDefaults& MeshResource::GetDefaults() const
	{
		return m_Description.Defaults;
	}
}
