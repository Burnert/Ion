#pragma once

#include "Resource.h"

namespace Ion
{
	struct ResourceControlBlock
	{
		Resource* ResourcePtr;
		size_t RefCount;

		ResourceControlBlock(Resource* resource) :
			ResourcePtr(resource),
			RefCount(0)
		{
		}
	};

	/**
	 * @brief Resource Manager class
	 * 
	 */
	class ION_API ResourceManager
	{
	public:
		/**
		 * @brief Get the Resource Manager singleton
		 * 
		 * @return Resource Manager object
		 */
		static ResourceManager& Get();

		template<typename T, TEnableIfT<TIsResourceV<T>>* = 0>
		static TResourceRef<T> Register(T* resource);
		static void Unregister(Resource* resource);

		template<typename T, TEnableIfT<TIsResourceV<T>>* = 0>
		static TResourceRef<T> FindAssociatedResource(const Asset& asset);
		static bool IsAnyResourceAvailable(const Asset& asset);

		template<typename T, TEnableIfT<TIsResourceV<T>>* = 0>
		static TArray<TResourceRef<T>> GetResourcesOfType();

		/**
		 * @brief Checks if the Asset is in use by a Resource
		 * 
		 * @param asset The Asset to check for
		 */
		static bool IsRegistered(Resource* resource);

		static inline constexpr size_t ResourceMapBucketCount = 256;

	private:
		ResourceManager();

		static ResourceControlBlock* GetControlBlock(Resource* resource) noexcept;

	private:
		THashMap<Resource*, ResourceControlBlock> m_Resources;
		THashMap<Asset, TArray<Resource*>> m_AssetToResources;

		static ResourceManager* s_Instance;

		friend class ResourceMemory;
		template<typename T>
		friend class TResoureRef;
		friend class _Detail::ResourceRefHelper;
	};

	// ResourceManager inline implementation

	template<typename T, TEnableIfT<TIsResourceV<T>>*>
	inline TResourceRef<T> ResourceManager::Register(T* resource)
	{
		ionassert(!IsRegistered(resource));

		ResourceManager& instance = Get();

		instance.m_Resources.emplace(resource, ResourceControlBlock(resource));

		const Asset& asset = resource->GetAssetHandle();
		// Associate the resource with the asset
		instance.m_AssetToResources[asset].push_back(resource);

		ResourceLogger.Trace("Registered resource \"{}\".", asset->GetVirtualPath());

		return TResourceRef<T>(resource);
	}

	template<typename T, TEnableIfT<TIsResourceV<T>>*>
	TResourceRef<T> ResourceManager::FindAssociatedResource(const Asset& asset)
	{
		ResourceManager& instance = Get();

		auto it = instance.m_AssetToResources.find(asset);
		if (it == instance.m_AssetToResources.end())
			return TResourceRef<T>();

		const TArray<Resource*>& resources = it->second;
		ionassert(!resources.empty());

		for (Resource* resource : resources)
		{
			if (T* castResource = dynamic_cast<T*>(resource))
			{
				return TResourceRef<T>(castResource);
			}
		}
		return TResourceRef<T>();
	}

	template<typename T, TEnableIfT<TIsResourceV<T>>*>
	inline TArray<TResourceRef<T>> ResourceManager::GetResourcesOfType()
	{
		ResourceManager& instance = Get();

		// @TODO: all the types should be indexed somewhere,
		// so that no searching is needed.

		TArray<TResourceRef<T>> resources;
	
		for (auto& [resource, block] : instance.m_Resources)
		{
			if (T* castResource = dynamic_cast<T*>(block.ResourcePtr))
			{
				resources.emplace_back(TResourceRef<T>(castResource));
			}
		}

		return resources;
	}

	inline ResourceControlBlock* ResourceManager::GetControlBlock(Resource* resource) noexcept
	{
		ResourceManager& instance = Get();

		auto it = instance.m_Resources.find(resource);
		if (it == instance.m_Resources.end())
			return nullptr;

		return &it->second;
	}
}
