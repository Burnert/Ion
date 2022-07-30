#pragma once

#include "AssetCommon.h"
#include "Core/Task/TaskQueue.h"

namespace Ion
{
	using TFuncAssetOnLoad = TFunction<void(const AssetData&)>;

	/**
	 * @brief Loads the asset from the file specified on construction.
	 */
	struct FAssetLoadWork : FTaskWork
	{
		// OnLoad Message
		TFuncAssetOnLoad OnLoad;
		// OnError Message
		TFunction<void(/*ErrorDesc*/)> OnError;

		FilePath AssetPath;
		EAssetType AssetType;
		bool bImportExternal;

		/**
		 * Sets this work's Execute function and sends the work to the AssetRegistry.
		 * The OnLoad and OnError functors must be set first.
		 * Call this instead of passing the object to AssetRegistry::ScheduleWork.
		 */
		void Schedule();
	};

	class ION_API AssetRegistry
	{
	public:
		/**
		 * @brief K - String - stores the virtual path of an asset.
		 * V - AssetDefinition - asset representation in memory.
		 */
		using AssetMap = THashMap<String, AssetDefinition>;

		/**
		 * @brief Registers an asset. Creates an AssetDefinition
		 * in the asset collection using the specified initializer.
		 * 
		 * @param initializer Asset description
		 * @return Reference to the asset definition
		 */
		static AssetDefinition& Register(const AssetInitializer& initializer);

		static void Unregister(const AssetDefinition& asset);

		/**
		 * @brief Find an asset definition by GUID.
		 * 
		 * @param guid GUID to use
		 * @return If found, the AssetDefinition pointer, else nullptr
		 */
		//static AssetDefinition* Find(const GUID& guid);

		/**
		 * @brief Find an asset definition by virtual path.
		 *
		 * @param virtualPath a VP to an asset (e.g. "<Engine>/Materials/DefaultMaterial")
		 * @return If found, the AssetDefinition pointer, else nullptr
		 */
		static AssetDefinition* Find(const String& virtualPath);

		static bool IsRegistered(const String& virtualPath);
		static bool IsRegistered(const Asset& asset);
		static bool IsRegistered(AssetDefinition* asset);

		/**
		 * @brief Add a work to the work queue, which will be executed
		 * by a free worker thread.
		 * 
		 * @tparam T Work type - must be FTaskWork or inherit from it
		 * @param work Work object
		 */
		template<typename T>
		static void ScheduleWork(T& work);

		/**
		 * Creates an array of handles to all the registered assets.
		 * Don't call it too many times.
		 */
		static TArray<Asset> GetAllRegisteredAssets();

		/**
		 * Creates an array of handles to all the registered assets of single type.
		 * Don't call it too many times.
		 * 
		 * @param type Asset type
		 */
		static TArray<Asset> GetAllRegisteredAssets(EAssetType type);

		static const AssetMap& GetAssetsMap();

		static void RegisterEngineAssets();

	private:
		AssetRegistry();

		static AssetRegistry& Get();

		AssetMap m_Assets;
		THashSet<AssetDefinition*> m_AssetPtrs;
		// @TODO: Variable memory pool allocator here

		/**
		 * @brief By default - the Engine Task Queue
		 */
		TaskQueue& m_WorkQueue;

		TShared<TTreeNode<FileInfo>> m_EngineContent;
	};

	// AssetRegistry class inline implementation ------------------------------

	template<typename T>
	inline void AssetRegistry::ScheduleWork(T& work)
	{
#if ION_DEBUG
		LOG_DEBUG("Work scheduled! {0}", work.GetDebugName());
#endif
		Get().m_WorkQueue.Schedule(work);
	}

	inline const AssetRegistry::AssetMap& AssetRegistry::GetAssetsMap()
	{
		return Get().m_Assets;
	}
}
