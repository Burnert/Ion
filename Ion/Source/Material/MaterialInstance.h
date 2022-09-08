#pragma once

#include "Material.h"

namespace Ion
{
#pragma region Material Parameter Instance

	class IMaterialParameterInstance
	{
	public:
		virtual IMaterialParameter* GetParameter() const = 0;
		inline EMaterialParameterType GetType() const;

		virtual ~IMaterialParameterInstance() { }

	private:
		void SetValue(const TMaterialParameterTypeVariant& value);

		friend class MaterialInstance;
	};

	inline EMaterialParameterType IMaterialParameterInstance::GetType() const
	{
		return GetParameter()->GetType();
	}

	// MaterialParameterInstance Implementations -------------------------------------------------------

	class MaterialParameterInstanceScalar : public IMaterialParameterInstance
	{
	public:
		MaterialParameterInstanceScalar(MaterialParameterScalar* parentParameter, float value);

		virtual IMaterialParameter* GetParameter() const override;

		virtual ~MaterialParameterInstanceScalar() override { }

		MaterialParameterScalar* GetParameterScalar() const;

		void SetValue(float value);
		float GetValue() const;

	private:
		MaterialParameterScalar* m_Parameter;
		float m_Value;
	};

	inline void MaterialParameterInstanceScalar::SetValue(float value)
	{
		m_Value = value;
	}

	inline float MaterialParameterInstanceScalar::GetValue() const
	{
		return m_Value;
	}

	class MaterialParameterInstanceVector : public IMaterialParameterInstance
	{
	public:
		MaterialParameterInstanceVector(MaterialParameterVector* parentParameter, const Vector4& value);

		virtual IMaterialParameter* GetParameter() const override;

		virtual ~MaterialParameterInstanceVector() override { }

		MaterialParameterVector* GetParameterVector() const;

		void SetValue(const Vector4& value);
		const Vector4& GetValue() const;

	private:
		MaterialParameterVector* m_Parameter;
		Vector4 m_Value;
	};

	inline void MaterialParameterInstanceVector::SetValue(const Vector4& value)
	{
		m_Value = value;
	}

	inline const Vector4& MaterialParameterInstanceVector::GetValue() const
	{
		return m_Value;
	}

	class MaterialParameterInstanceTexture2D : public IMaterialParameterInstance
	{
	public:
		MaterialParameterInstanceTexture2D(MaterialParameterTexture2D* parentParameter, Asset value);

		virtual IMaterialParameter* GetParameter() const override;

		virtual ~MaterialParameterInstanceTexture2D() override { }

		MaterialParameterTexture2D* GetParameterTexture2D() const;

		void SetValue(Asset value);
		Asset GetValue() const;

		void Bind(uint32 slot);

	private:
		void UpdateTexture();

	private:
		MaterialParameterTexture2D* m_Parameter;
		Asset m_Value;

		TSharedPtr<TextureResource> m_TextureResource;
		TRef<RHITexture> m_Texture;
	};

	inline void MaterialParameterInstanceTexture2D::SetValue(Asset value)
	{
		m_Value = value;
		UpdateTexture();
	}

	inline Asset MaterialParameterInstanceTexture2D::GetValue() const
	{
		return m_Value;
	}

#pragma endregion

#pragma region Material Instance Asset Type

	class MaterialInstanceAssetType : public IAssetType
	{
	public:
		virtual Result<TSharedPtr<IAssetCustomData>, IOError> Parse(const std::shared_ptr<XMLDocument>& xml) const override;
		ASSET_TYPE_NAME_IMPL("Ion.MaterialInstance")
	};

	REGISTER_ASSET_TYPE_CLASS(MaterialInstanceAssetType);

	class MaterialInstanceAssetData : public IAssetCustomData
	{
	public:
		struct Parameter
		{
			MaterialInstanceAssetParameterInstanceValues Values;
			String Name;
			EMaterialParameterType Type;
		};

		ASSET_DATA_GETTYPE_IMPL(AT_MaterialInstanceAssetType)

		String ParentMaterialAssetVP;
		TArray<Parameter> Parameters;
	};

#pragma endregion

#pragma region Material Instance

	class ION_API MaterialInstance
	{
	public:
		static std::shared_ptr<MaterialInstance> Create(const std::shared_ptr<Material>& parentMaterial);
		static std::shared_ptr<MaterialInstance> CreateFromAsset(Asset materialInstanceAsset);

		void BindTextures() const;

		IMaterialParameterInstance* GetMaterialParameterInstance(const String& name) const;

		template<typename T>
		T* GetMaterialParameterInstanceTyped(const String& name) const;

		const std::shared_ptr<Material>& GetBaseMaterial() const;

		/**
		 * @brief Set the values in the associated material uniform buffer
		 * to the parameter instance values
		 */
		void TransferParameters() const;

		Asset GetAsset() const;

		~MaterialInstance();

	private:
		MaterialInstance(const std::shared_ptr<Material>& parentMaterial);
		MaterialInstance(Asset materialInstanceAsset);

		void SetParentMaterial(const std::shared_ptr<Material>& material);

		void CreateParameterInstances();
		void DestroyParameterInstances();

	private:
		std::shared_ptr<Material> m_ParentMaterial;

		THashMap<String, IMaterialParameterInstance*> m_ParameterInstances;
		THashSet<MaterialParameterInstanceTexture2D*> m_TextureParameterInstances;

		Asset m_Asset;
	};

	template<typename T>
	inline T* MaterialInstance::GetMaterialParameterInstanceTyped(const String& name) const
	{
		ionassert(dynamic_cast<T*>(GetMaterialParameterInstance(name)));
		return (T*)GetMaterialParameterInstance(name);
	}

	inline const std::shared_ptr<Material>& MaterialInstance::GetBaseMaterial() const
	{
		return m_ParentMaterial;
	}

	inline Asset MaterialInstance::GetAsset() const
	{
		return m_Asset;
	}

#pragma endregion
}
