#pragma once

#include "AssetCommon.h"
#include "Asset.h"

namespace Ion
{
	struct AssetImportData
	{
		FilePath Path;
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
		/**
		 * @brief Get the Handle for this AssetDefinition object
		 * 
		 * @return Asset handle
		 */
		Asset GetHandle() const;

		/**
		 * @brief Loads the file specified in the <ImportExternal> node in the asset file.
		 * 
		 * @tparam FImport lambda void(TShared<AssetFileMemoryBlock>)
		 * @tparam FReady  lambda void()
		 * 
		 * @param onImport Function called on a worker thread after the file has been loaded.
		 * Used to reinterpret the file from a raw imported format to data that can be used to initialize some resource.
		 * 
		 * @param onReady Function called on the main thread, always after onImport.
		 * Used to initialize some resource using the imported data.
		 */
		template<typename FImport, typename FReady>
		void Import(FImport onImport, FReady onReady);

		EAssetType GetType() const;

		/**
		 * @brief Returns the path specified in the <ImportExternal> node.
		 */
		const FilePath& GetImportPath() const;

		/**
		 * @brief Returns the path of the .iasset file.
		 */
		const FilePath& GetDefinitionPath() const;

		/**
		 * @brief Returns the virtual path of the asset.
		 * e.g. [Engine]/Materials/DefaultMaterial
		 */
		const String& GetVirtualPath() const;

		const AssetInfo& GetInfo() const;

		const GUID& GetGuid() const;

		~AssetDefinition();

		AssetDefinition(const AssetDefinition&) = default;
		AssetDefinition(AssetDefinition&&) = default;

		AssetDefinition& operator=(const AssetDefinition&) = default;
		AssetDefinition& operator=(AssetDefinition&&) = default;

	private:
		explicit AssetDefinition(const AssetInitializer& initializer);

		bool ParseAssetDefinitionFile(const FilePath& path);

	private:
		GUID m_Guid;
		String m_VirtualPath;

		/**
		 * @brief Path of the loaded .iasset file.
		 */
		FilePath m_AssetDefinitionPath;

		/**
		 * @brief Path specified in the asset definition file.
		 */
		FilePath m_AssetReferencePath;

		AssetInfo m_Info;

		EAssetType m_Type;

		/**
		 * @brief Whether the asset is an external, non-native file,
		 * that has to be imported before use.
		 */
		uint8 m_bImportExternal : 1;

		friend class AssetRegistry;
	};

	// AssetDefinition class inline implementation --------------------------------

	inline Asset AssetDefinition::GetHandle() const
	{
		return Asset(const_cast<AssetDefinition*>(this));
	}

	template<typename FImport, typename FReady>
	inline void AssetDefinition::Import(FImport onImport, FReady onReady)
	{
		static_assert(TIsConvertibleV<FImport, TFunction<void(TShared<AssetFileMemoryBlock>)>>);
		static_assert(TIsConvertibleV<FReady, TFunction<void()>>);

		ionassert(Platform::IsMainThread());
		ionassert(m_AssetReferencePath.IsFile());

		AssetImportData importData { m_AssetReferencePath };

		AsyncTask([onImport, onReady, importData](IMessageQueueProvider& q)
		{
			// Worker thread:
			TShared<AssetFileMemoryBlock> data(new AssetFileMemoryBlock, [](AssetFileMemoryBlock* ptr)
			{
				ptr->Free();
				delete ptr;
			});

			File assetFile(importData.Path, EFileMode::Read);
			data->Count = assetFile.GetSize();
			data->Ptr = new uint8[data->Count];
			if (!assetFile.Read(data->Ptr, data->Count))
				return;

			onImport(data);

			q.PushMessage(FTaskMessage([onReady]
			{
				// Main thread:

				onReady();
			}));
		}).Schedule();
	}

	inline EAssetType AssetDefinition::GetType() const
	{
		return m_Type;
	}

	inline const FilePath& AssetDefinition::GetImportPath() const
	{
		return m_AssetReferencePath;
	}

	inline const FilePath& AssetDefinition::GetDefinitionPath() const
	{
		return m_AssetDefinitionPath;
	}

	inline const String& AssetDefinition::GetVirtualPath() const
	{
		return m_VirtualPath;
	}

	inline const AssetInfo& AssetDefinition::GetInfo() const
	{
		return m_Info;
	}

	inline const GUID& AssetDefinition::GetGuid() const
	{
		return m_Guid;
	}
}
