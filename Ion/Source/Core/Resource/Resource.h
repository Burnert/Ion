#pragma once

#include "Core/Asset/Asset.h"
#include "Core/Asset/AssetRegistry.h"

namespace Ion
{
	// ------------------------------------------------------------
	// Base Resource
	// ------------------------------------------------------------

	template<typename T>
	using TFuncResourceOnTake = TFunction<void(TShared<T>)>;

	/**
	 * @brief Base Resource class
	 */
	class ION_API Resource
	{
	public:
		~Resource();

	protected:
		Resource(const Asset& asset);

		/**
		 * @brief Query the Resource Manager for a Resource
		 * of type T associated with the specified asset.
		 * 
		 * @tparam T Resource subtype
		 * @param asset Asset associated with the Resource
		 * @return Shared pointer to the Resource
		 */
		template<typename T>
		static TShared<T> Query(const Asset& asset);

	protected:
		Asset m_Asset;

		friend class ResourceManager;
	};

	template<typename T>
	inline TShared<T> Resource::Query(const Asset& asset)
	{
		static_assert(TIsBaseOfV<Resource, T>);

		if (TShared<Resource> resource = ResourceManager::Find(asset))
		{
			return TStaticCast<T>(resource);
		}
		else
		{
			// Register the new resource, if it doesn't exist.
			TShared<T> newResource = MakeShareable(new T(asset));
			ResourceManager::Register(asset.GetGuid(), newResource);
			return newResource;
		}
	}

	// ------------------------------------------------------------
	// Texture Resource
	// ------------------------------------------------------------

	class ION_API TextureResource : public Resource
	{
	public:
		/**
		 * @brief Query the Resource Manager for a Texture Resource
		 * associated with the specified asset.
		 * 
		 * @see Resource::Query(const Asset& asset)
		 * 
		 * @param asset Asset associated with the Resource
		 * @return Shared pointer to the Resource
		 */
		static TShared<TextureResource> Query(const Asset& asset);

		/**
		 * @brief Used to access the Texture owned by the Resource.
		 * 
		 * @tparam Lambda Must be convertible to TFuncResourceOnTake<Texture>
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
		TextureResource(const Asset& asset) :
			Resource(asset)
		{
		}

	private:
		TShared<RHITexture> m_Texture;

		friend class Resource;
	};

	template<typename Lambda>
	inline bool TextureResource::Take(Lambda onTake)
	{
		if (m_Texture)
		{
			onTake(m_Texture);
			return true;
		}

		auto initTexture = [this, onTake](const AssetData& data)
		{
			ionassert(m_Asset->GetType() == EAssetType::Image);

			TShared<Image> image = data.Get<Image>();

			TextureDescription desc { };
			desc.Dimensions.Width = image->GetWidth();
			desc.Dimensions.Height = image->GetHeight();
			desc.bGenerateMips = true;
			desc.bCreateSampler = true;
			desc.bUseAsRenderTarget = true;
			desc.DebugName = StringConverter::WStringToString(m_Asset->GetDefinitionPath().ToString());

			m_Texture = RHITexture::Create(desc);

			onTake(m_Texture);
		};

		TOptional<AssetData> data = m_Asset->Load(initTexture);

		if (data)
		{
			initTexture(data.value());
			return true;
		}

		return false;
	}

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
