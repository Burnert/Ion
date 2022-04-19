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
		TFuncAssetOnLoad OnLoad;
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

	/**
	 * @brief Asset definition class
	 *
	 * @details The asset definition is a representation of an .iasset file.
	 * It is used to defer loading the resources (assets) until they are needed by some
	 * part of the engine, for example to create a Mesh Component or a Texture object.
	 */
	class ION_API AssetDefinition
	{
	public:
		Asset GetHandle() const;

		/**
		 * @brief Loads the asset specified in the asset definition file
		 * into the asset pool, or returns the existing data.
		 *
		 * @param onLoad Lambda that will be executed once the asset is fully
		 * loaded. Has to be of type @code (AssetData) -> void @endcode
		 *
		 * @return If the asset is already loaded, an optional of AssetData,
		 * containing the pointer to the raw asset data and its size.
		 * If it's not, an empty optional.
		 */
		template<typename Lambda>
		TOptional<AssetData> Load(Lambda onLoad);

		EAssetType GetType() const;

		/**
		 * @brief Checks if the path specified in the .iasset file
		 * is valid, and whether the asset can be loaded from the file.
		 */
		bool IsValid() const;

		~AssetDefinition();

		/** @brief Same as IsValid */
		operator bool() const;

		AssetDefinition(const AssetDefinition& other) = default;
		AssetDefinition(AssetDefinition&& other) = default;

		AssetDefinition& operator=(const AssetDefinition& other) = default;
		AssetDefinition& operator=(AssetDefinition&& other) = default;

	private:
		explicit AssetDefinition(const AssetInitializer& initializer);

		void ParseAssetDefinitionFile();

	private:
		GUID m_Guid;
		/** @brief Path of the loaded .iasset file. */
		FilePath m_AssetDefinitionPath;
		/** @brief Path specified in the asset definition file. */
		FilePath m_AssetReferencePath;

		AssetData m_AssetData;

		EAssetType m_Type;
		/**
		 * @brief Whether the asset is an external, non-native file,
		 * that has to be imported before use.
		 */
		uint8 m_bImportExternal : 1;
		
		bool m_bIsLoaded;

		friend class AssetRegistry;
		friend TFuncAssetOnLoad;
	};

	class ION_API AssetRegistry
	{
	public:
		using AssetMap = THashMap<GUID, AssetDefinition>;

		/**
		 * @brief Registers an asset. Creates an AssetDefinition
		 * in the asset collection using the specified initializer.
		 * 
		 * @param initializer Asset description
		 * @return Reference to the asset definition
		 */
		static AssetDefinition& Register(const AssetInitializer& initializer);

		/**
		 * @brief Find an asset definition by GUID.
		 * 
		 * @param guid GUID to use
		 * @return If found, the AssetDefinition pointer, else nullptr
		 */
		static AssetDefinition* Find(const GUID& guid);

		/**
		 * @brief Add a work to the work queue, which will be executed
		 * by a free worker thread.
		 * 
		 * @tparam T Work type - must be FTaskWork or inherit from it
		 * @param work Work object
		 */
		template<typename T>
		static void ScheduleWork(T& work);

		static const AssetMap& GetAllRegisteredAssets();

	private:
		AssetRegistry();

		static AssetRegistry& Get();

		AssetMap m_Assets;
		// @TODO: Variable memory pool allocator here

		/**
		 * @brief By default - the Engine Task Queue
		 */
		TaskQueue& m_WorkQueue;
	};

	// AssetDefinition class inline implementation --------------------------------

	template<typename Lambda>
	inline TOptional<AssetData> AssetDefinition::Load(Lambda onLoad)
	{
		static_assert(TIsConvertibleV<Lambda, TFuncAssetOnLoad>);

		ionassertnd(IsValid());

		// Return right away if the asset is loaded
		if (m_bIsLoaded)
		{
			return m_AssetData;
		}

		// Prepare the work
		FAssetLoadWork work;
		work.AssetPath = m_AssetReferencePath;
		work.AssetType = m_Type;
		work.bImportExternal = m_bImportExternal;
		work.OnLoad = [guid = this->m_Guid, onLoad](const AssetData& data) mutable
		{
			AssetDefinition* asset = AssetRegistry::Find(guid);

			asset->m_AssetData = Move(data);
			onLoad(asset->m_AssetData);
		};
		work.OnError = []
		{
			LOG_ERROR("Error!");
		};
		work.Schedule();

		return NullOpt;
	}

	inline EAssetType AssetDefinition::GetType() const
	{
		return m_Type;
	}

	inline bool AssetDefinition::IsValid() const
	{
		return m_AssetReferencePath.IsFile();
	}

	inline AssetDefinition::operator bool() const
	{
		return IsValid();
	}

	// AssetRegistry class inline implementation ------------------------------

	template<typename T>
	inline void AssetRegistry::ScheduleWork(T& work)
	{
		LOG_INFO("Work scheduled!");
		Get().m_WorkQueue.Schedule(work);
	}

	inline const AssetRegistry::AssetMap& AssetRegistry::GetAllRegisteredAssets()
	{
		return Get().m_Assets;
	}
}
