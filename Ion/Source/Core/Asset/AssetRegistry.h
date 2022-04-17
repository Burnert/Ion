#pragma once

#include "AssetCommon.h"

namespace Ion
{
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

		void LoadData();

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
	};

	class ION_API AssetRegistry
	{
	public:
		using AssetMap = THashMap<GUID, AssetDefinition>;

		static AssetDefinition& Register(const AssetInitializer& initializer);

		static AssetDefinition* Find(const GUID& guid);

		static const AssetMap& GetAllRegisteredAssets();

	private:
		static AssetRegistry& Get();

		AssetMap m_Assets;
		// @TODO: Variable memory pool allocator here
	};

	class ION_API AssetWorker
	{
	public:
		AssetWorker();

	private:
		void Start();
		void Exit();

		// Worker Thread Functions ------------------------------------------------

		void WorkerProc();

		friend class AssetRegistry;
	};

	// AssetDefinition class inline implementation --------------------------------

	template<typename Lambda>
	inline TOptional<AssetData> AssetDefinition::Load(Lambda onLoad)
	{
		static_assert(TIsConvertibleV<Lambda, TFunction<void(AssetData)>>);

		ionassertnd(IsValid());

		if (m_bIsLoaded)
		{
			TOptional<AssetData>();
		}

		LoadData();

		return TOptional<AssetData>();
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

	inline const AssetRegistry::AssetMap& AssetRegistry::GetAllRegisteredAssets()
	{
		return Get().m_Assets;
	}
}
