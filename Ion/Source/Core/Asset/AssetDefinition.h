#pragma once

#include "AssetCommon.h"
#include "Core/Task/TaskQueue.h"

namespace Ion
{
	/**
 * @brief Asset properties (fields) that can be accessed by multiple threads
 */
	struct AssetProps
	{
		bool bLoaded;

		AssetProps() :
			bLoaded(false)
		{
		}
	};

	/**
	 * @brief AssetProps structure with a mutex
	 */
	struct AssetPropsThreadSafe : AssetProps
	{
		/**
		 * @brief Construct a new Asset Props Thread Safe object
		 */
		AssetPropsThreadSafe() { }

		/**
		 * @brief Thread safe copy constructor
		 *
		 * @param other AssetPropsThreadSafe object to copy from
		 */
		AssetPropsThreadSafe(const AssetPropsThreadSafe& other) :
			AssetProps()
		{
			// Current thread has to own the mutex before copying
			UniqueLock lock(other.m_PropsMutex);
			bLoaded = other.bLoaded;
		}

	private:
		mutable Mutex m_PropsMutex;

		template<typename T>
		friend struct TAssetPropsScopedAccessor;
	};

	/**
	 * @brief A primitive used for accessing the AssetProps structure
	 * in the AssetDefinition class in a thread safe manner.
	 *
	 * @details Automatically locks the mutex on construction
	 * and unlocks it on destruction.
	 *
	 * @tparam T AssetDefinition class (const or non-const)
	 */
	template<typename T>
	struct TAssetPropsScopedAccessor
	{
		static_assert(TIsSameV<TRemoveConst<T>, AssetDefinition>);
		using TPropsType = TIf<TIsConstV<T>, const AssetPropsThreadSafe, AssetPropsThreadSafe>;

		/**
		 * @brief Construct a new TAssetPropsScopedAccessor object
		 *
		 * @details By default, tries to lock the mutex, and if it does not
		 * succeed, the props cannot be accessed. If the flag is specified,
		 * it waits for the mutex to be available.
		 * If multiple properties have to be set throughout a long timespan,
		 * it is adviced to first, gather all the data, and then access
		 * the props and set all of them at the same time.
		 * The same applies for getting the props.
		 *
		 * @param assetDefinition AssetDefinition object to access
		 * @param bWait Whether to block the thread if the mutex cannot be owned
		 */
		TAssetPropsScopedAccessor(T& assetDefinition, bool bWait = false);

		/**
		 * @brief Acces the props. Don't call it if the props
		 * could not be accessed by the current thread.
		 *
		 * @return Asset Props reference
		 */
		inline TPropsType& Access() const
		{
			ionassertnd(m_Props);
			return *m_Props;
		}

		/**
		 * @brief Same as Access()
		 */
		inline TPropsType* operator->() const
		{
			ionassertnd(m_Props);
			return m_Props;
		}

		/**
		 * @brief Checks if the props can be accessed
		 * by the current thread.
		 */
		inline operator bool() const
		{
			return m_Props;
		}

	private:
		TPropsType* m_Props;
		UniqueLock m_Lock;
	};

	// TAssetPropsScopedAccessor inline implementation -----------------------------------

	template<typename T>
	FORCEINLINE TAssetPropsScopedAccessor<T>::TAssetPropsScopedAccessor(T& assetDefinition, bool bWait) :
		m_Lock(assetDefinition.m_Props.m_PropsMutex, LockProp::DeferLock),
		m_Props(nullptr)
	{
		// Props can't be accessed by multiple threads
		if (bWait)
		{
			m_Lock.lock();
		}
		else if (!m_Lock.try_lock())
		{
			return;
		}
		m_Props = &assetDefinition.m_Props;
	}

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
		Asset GetHandle() const;

		/**
		 * @brief Loads the asset specified in the asset definition file
		 * into the asset pool, or returns the existing data.
		 *
		 * @param onLoad Lambda that will be executed once the asset is fully
		 * loaded. Has to be of type @code (const AssetData&) -> void @endcode
		 *
		 * @see TFuncAssetOnLoad
		 *
		 * @return If the asset is already loaded, an optional of AssetData,
		 * containing the pointer to the raw asset data and its size.
		 * If it's not, an empty optional.
		 */
		template<typename Lambda>
		TOptional<AssetData> Load(Lambda onLoad);

		template<typename FImport, typename FReady>
		void Import(FImport onImport, FReady onReady);

		EAssetType GetType() const;

		const FilePath& GetPath() const;
		const FilePath& GetDefinitionPath() const;
		const String& GetVirtualPath() const;

		const AssetInfo& GetInfo() const;

		const GUID& GetGuid() const;

		/**
		 * @brief Checks if the path specified in the .iasset file
		 * is valid, and whether the asset can be loaded from the file.
		 */
		bool IsValid() const;

		bool IsLoaded() const;

		~AssetDefinition();

		/** @brief Same as IsValid */
		operator bool() const;

		AssetDefinition(const AssetDefinition& other) = default;
		AssetDefinition(AssetDefinition&& other) = default;

		AssetDefinition& operator=(const AssetDefinition& other) = default;
		AssetDefinition& operator=(AssetDefinition&& other) = default;

	private:
		explicit AssetDefinition(const AssetInitializer& initializer);

		bool ParseAssetDefinitionFile(const TShared<XMLDocument>& xml);

	private:
		GUID m_Guid;
		String m_VirtualPath;
		/** @brief Path of the loaded .iasset file. */
		FilePath m_AssetDefinitionPath;
		/** @brief Path specified in the asset definition file. */
		FilePath m_AssetReferencePath;

		AssetInfo m_Info;

		AssetData m_AssetData;

		EAssetType m_Type;
		/**
		 * @brief Whether the asset is an external, non-native file,
		 * that has to be imported before use.
		 */
		uint8 m_bImportExternal : 1;

		AssetPropsThreadSafe m_Props;

		friend class AssetRegistry;
		template<typename T>
		friend struct TAssetPropsScopedAccessor;
		//friend TFuncAssetOnLoad;
	};

	// AssetDefinition class inline implementation --------------------------------

	template<typename Lambda>
	inline TOptional<AssetData> AssetDefinition::Load(Lambda onLoad)
	{
		// @TODO: When a call to this function is made while something's already scheduled the work,
		// another work gets scheduled. This should not be the case. The second call should just
		// wait for the asset and get notified when it finally loads.

		static_assert(TIsConvertibleV<Lambda, TFuncAssetOnLoad>);

		ionassertnd(IsValid());

		// Return right away if the asset is loaded
		bool bLoaded = false;
		{
			TAssetPropsScopedAccessor accessor(*this);
			bLoaded = accessor->bLoaded;
		}
		if (bLoaded)
		{
			return m_AssetData;
		}

		// Prepare the work
		FAssetLoadWork work;
		work.AssetPath = m_AssetReferencePath;
		work.AssetType = m_Type;
		work.bImportExternal = m_bImportExternal;
		work.OnLoad = [path = m_AssetDefinitionPath, handle = GetHandle(), onLoad](const AssetData& data) mutable
		{
			AssetDefinition* asset = handle.GetAssetDefinition();

			if (!asset)
			{
				LOG_ERROR(L"Asset \"{0}\" had already been deleted before the data could be loaded.", path.ToString());
				return;
			}

			asset->m_AssetData = Move(data);
			{
				TAssetPropsScopedAccessor props(*asset);
				props->bLoaded = true;
			}
			onLoad(asset->m_AssetData);
		};
		work.OnError = []
		{
			LOG_ERROR("Error!");
		};
		work.Schedule();

		return NullOpt;
	}

	template<typename FImport, typename FReady>
	inline void AssetDefinition::Import(FImport onImport, FReady onReady)
	{
		static_assert(TIsConvertibleV<FImport, TFunction<void(TShared<AssetFileMemoryBlock>)>>);

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

	inline const FilePath& AssetDefinition::GetPath() const
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

	inline bool AssetDefinition::IsValid() const
	{
		return m_AssetReferencePath.IsFile();
	}

	inline bool AssetDefinition::IsLoaded() const
	{
		bool bLoaded = false;
		{
			TAssetPropsScopedAccessor accessor(*this);
			bLoaded = accessor->bLoaded;
		}
		return bLoaded;
	}

	inline AssetDefinition::operator bool() const
	{
		return IsValid();
	}
}