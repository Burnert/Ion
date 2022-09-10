#pragma once

#include "AssetCommon.h"
#include "Asset.h"

namespace Ion
{
	struct AssetImportData
	{
		FilePath Path;
	};

	// @TODO: TEMPORARY
	namespace Editor { class EditorApplication; }

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
		 * @tparam FImport lambda std::shared_ptr<TData>(std::shared_ptr<AssetFileMemoryBlock>)
		 * @tparam FReady  lambda void(std::shared_ptr<TData>)
		 * @tparam FError  lambda void(auto& result)
		 * 
		 * @param onImport Function called on a worker thread after the file has been loaded.
		 * Used to reinterpret the file from a raw imported format to data that can be used to initialize some resource.
		 * 
		 * @param onReady Function called on the main thread, always after onImport.
		 * Used to initialize some resource using the imported data.
		 * 
		 * @param onError If specified, function called on the main thread if an error occured during importing.
		 * The result argument is always an Error variant.
		 */
		template<typename FImport, typename FReady, typename FError>
		void Import(FImport onImport, FReady onReady, FError onError = nullptr);

		IAssetType& GetType() const;

		TSharedPtr<IAssetCustomData> GetCustomData() const;

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

		Result<void, IOError> ParseAssetDefinitionFile(const std::shared_ptr<XMLDocument>& xml);

		Result<void, IOError> Serialize(Archive& ar);

	private:
		GUID m_Guid;
		String m_VirtualPath;

		/**
		 * @brief Path of the loaded .iasset file.
		 */
		FilePath m_AssetDefinitionPath;

		/**
		 * @brief Import external path specified in the asset definition file.
		 */
		FilePath m_AssetImportPath;

		AssetInfo m_Info;

		IAssetType* m_Type;
		TSharedPtr<IAssetCustomData> m_CustomData;

		/**
		 * @brief Whether the asset is an external, non-native file,
		 * that has to be imported before use.
		 */
		uint8 m_bImportExternal : 1;

		friend class AssetRegistry;
		friend class IAssetType;
		// @TODO: TEMPORARY
		friend class Editor::EditorApplication;
	};

	// AssetDefinition class inline implementation --------------------------------

	inline Asset AssetDefinition::GetHandle() const
	{
		return Asset(const_cast<AssetDefinition*>(this));
	}

	template<typename FImport, typename FReady, typename FError>
	inline void AssetDefinition::Import(FImport onImport, FReady onReady, FError onError)
	{
		using TImportRet = decltype(onImport(nullptr));
		static constexpr bool bReportError = TIsDifferentV<FError, nullptr_t>;

		static_assert(TIsSharedV<TImportRet>, "The type returned from onImport must be a shared pointer.");
		static_assert(TIsConvertibleV<FImport, TFunction<TImportRet(std::shared_ptr<AssetFileMemoryBlock>)>>);
		static_assert(TIsConvertibleV<FReady, TFunction<void(TImportRet)>>,
			"onReady argument type and onImport return type must be the same.");

		ionassert(Platform::IsMainThread(), "Asset import function can be called only on the main thread.");
		ionassert(m_AssetImportPath.IsFile());

		AssetImportData importData { m_AssetImportPath };

		AsyncTask([onImport, onReady, onError, importData](IMessageQueueProvider& q)
		{
			// Worker thread:
			std::shared_ptr<AssetFileMemoryBlock> data(new AssetFileMemoryBlock, [](AssetFileMemoryBlock* ptr)
			{
				ptr->Free();
				delete ptr;
			});

			File assetFile(importData.Path);
			auto openResult = assetFile.Open();
			if (!openResult)
			{
				if constexpr (bReportError)
				{
					q.PushMessage(FTaskMessage([onError, openResult = Move(openResult)]
					{
						onError(openResult);
					}));
				}
				return;
			}
			data->Count = assetFile.GetSize();
			data->Ptr = new uint8[data->Count];
			auto readResult = assetFile.Read(data->Ptr, data->Count);
			if (!readResult)
			{
				if constexpr (bReportError)
				{
					q.PushMessage(FTaskMessage([onError, readResult = Move(readResult)]
					{
						onError(readResult);
					}));
				}
				return;
			}

			// onImport function should return a value that will be used in the
			// onReady function on the main thread to initialize some object.
			auto imported = onImport(data);

			// Execute onReady on the main thread.
			q.PushMessage(FTaskMessage([onReady, imported]
			{
				onReady(imported);
			}));
		}).Schedule();

		AssetLogger.Trace("Asset \"{}\" import task has been scheduled.", m_VirtualPath);
	}

	inline IAssetType& AssetDefinition::GetType() const
	{
		ionassert(m_Type);
		return *m_Type;
	}

	inline TSharedPtr<IAssetCustomData> AssetDefinition::GetCustomData() const
	{
		return m_CustomData;
	}

	inline const FilePath& AssetDefinition::GetImportPath() const
	{
		return m_AssetImportPath;
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
