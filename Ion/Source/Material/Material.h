#pragma once

#include "MaterialCommon.h"
#include "Core/Core.h"
#include "Core/Asset/Asset.h"
#include "Core/File/XML.h"

#include "Resource/TextureResource.h"

namespace Ion
{
	enum class EShaderUsage : uint8
	{
		None = 0,
		StaticMesh   = Bitflag(0),
		SkeletalMesh = Bitflag(1),
		PostProcess  = Bitflag(2)
	};

	struct ShaderPermutation
	{
		TShared<RHIShader> Shader;
		EShaderUsage Usage;
		bool bCompiled;

		ShaderPermutation(EShaderUsage usage);
	};

	enum class EMaterialParameterType
	{
		Null = 0,
		Scalar,
		Vector,
		Texture2D
	};

#define _MPTFSHelper(type) if (strcmp(str, #type) == 0) return EMaterialParameterType::type
	static EMaterialParameterType MaterialParameterTypeFromString(const char* str)
	{
		_MPTFSHelper(Scalar);
		_MPTFSHelper(Vector);
		_MPTFSHelper(Texture2D);
		return EMaterialParameterType::Null;
	}

	// IMaterialParameter Interface -------------------------------------------------------

	class IMaterialParameter
	{
	public:
		virtual EMaterialParameterType GetType() const = 0;

		virtual ~IMaterialParameter() { }

		friend class Material;
	};

	// MaterialParameter Implementations -------------------------------------------------------

	class MaterialParameterScalar : public IMaterialParameter
	{
	public:
		MaterialParameterScalar(
			float defaultValue = 0.0f,
			float min = TNumericLimits<float>::lowest(),
			float max = TNumericLimits<float>::max()
		);

		virtual EMaterialParameterType GetType() const override;

		virtual ~MaterialParameterScalar() override { }

		float GetDefaultValue() const;
		float GetMinValue() const;
		float GetMaxValue() const;

	private:
		void SetDefaultValue(float value);
		void SetMinValue(float min);
		void SetMaxValue(float max);

	private:
		float m_DefaultValue;
		float m_MinValue;
		float m_MaxValue;

		friend class Material;
	};

	inline float MaterialParameterScalar::GetDefaultValue() const
	{
		return m_DefaultValue;
	}

	inline float MaterialParameterScalar::GetMinValue() const
	{
		return m_MinValue;
	}

	inline float MaterialParameterScalar::GetMaxValue() const
	{
		return m_MaxValue;
	}

	inline void MaterialParameterScalar::SetDefaultValue(float value)
	{
		m_DefaultValue = value;
	}

	inline void MaterialParameterScalar::SetMinValue(float min)
	{
		m_MinValue = min;
	}

	inline void MaterialParameterScalar::SetMaxValue(float max)
	{
		m_MaxValue = max;
	}

	class MaterialParameterVector : public IMaterialParameter
	{
	public:
		MaterialParameterVector(
			Vector4 defaultValue = Vector4(0.0f),
			Vector4 min = Vector4(TNumericLimits<float>::lowest()),
			Vector4 max = Vector4(TNumericLimits<float>::max())
		);

		virtual EMaterialParameterType GetType() const override;

		virtual ~MaterialParameterVector() override { }

		const Vector4& GetDefaultValue() const;
		const Vector4& GetMinValue() const;
		const Vector4& GetMaxValue() const;

	private:
		void SetDefaultValue(const Vector4& value);
		void SetMinValue(const Vector4& min);
		void SetMaxValue(const Vector4& max);

	private:
		Vector4 m_DefaultValue;
		Vector4 m_MinValue;
		Vector4 m_MaxValue;

		friend class Material;
	};

	inline const Vector4& MaterialParameterVector::GetDefaultValue() const
	{
		return m_DefaultValue;
	}

	inline const Vector4& MaterialParameterVector::GetMinValue() const
	{
		return m_MinValue;
	}

	inline const Vector4& MaterialParameterVector::GetMaxValue() const
	{
		return m_MaxValue;
	}

	inline void MaterialParameterVector::SetDefaultValue(const Vector4& value)
	{
		m_DefaultValue = value;
	}

	inline void MaterialParameterVector::SetMinValue(const Vector4& min)
	{
		m_MinValue = min;
	}

	inline void MaterialParameterVector::SetMaxValue(const Vector4& max)
	{
		m_MaxValue = max;
	}

	class MaterialParameterTexture2D : public IMaterialParameter
	{
	public:
		MaterialParameterTexture2D(uint32 slot = 0, Asset defaultValue = Asset());

		virtual EMaterialParameterType GetType() const override;

		virtual ~MaterialParameterTexture2D() override { }

		uint32 GetSlot() const;

		Asset GetDefaultValue() const;

	private:
		void SetDefaultValue(Asset value);

	private:
		Asset m_DefaultValue;
		uint32 m_TextureSlot;

		friend class Material;
	};

	inline uint32 MaterialParameterTexture2D::GetSlot() const
	{
		return m_TextureSlot;
	}

	inline Asset MaterialParameterTexture2D::GetDefaultValue() const
	{
		return m_DefaultValue;
	}

	inline void MaterialParameterTexture2D::SetDefaultValue(Asset value)
	{
		m_DefaultValue = value;
	}

	// IMaterialParameterInstance -------------------------------------------------------

	class IMaterialParameterInstance
	{
	public:
		virtual IMaterialParameter* GetParameter() const = 0;
		inline EMaterialParameterType GetType() const
		{
			return GetParameter()->GetType();
		}

		virtual ~IMaterialParameterInstance() { }

		friend class Material;
	};

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

	private:
		MaterialParameterTexture2D* m_Parameter;
		Asset m_Value;
	};

	inline void MaterialParameterInstanceTexture2D::SetValue(Asset value)
	{
		m_Value = value;
	}

	inline Asset MaterialParameterInstanceTexture2D::GetValue() const
	{
		return m_Value;
	}

	// Material Class -------------------------------------------------------------------

	class MaterialInstance;

	class ION_API Material : public std::enable_shared_from_this<Material>
	{
	public:
		static TShared<Material> Create();
		static TShared<Material> CreateFromAsset(Asset materialAsset);

		void CompileShaders();

		void AddUsage(EShaderUsage usage);
		void RemoveUsage(EShaderUsage usage);
		bool IsUsableWith(EShaderUsage usage);

		void SetMaterialCode(const String& code);

		void Invalidate();

		bool CompileMaterialCode();
		bool CompileMaterialCode(EShaderUsage usage);

	private:
		Material();
		Material(Asset materialAsset);

		bool ParseAsset(Asset materialAsset);
		bool ParseMaterialParameter(XMLNode* parameterNode, const FilePath& path);
		bool ParseMaterialCode(const String& code);
		bool LoadExternalMaterialCode(const FilePath& path);

		IMaterialParameter* AddParameter(const String& name, EMaterialParameterType type);
		bool RemoveParameter(const String& name);

		uint32 GetTextureParameterCount() const;
		uint32 GetFirstFreeTextureSlot() const;

		bool CompileMaterialCode_Internal(EShaderUsage usage, ShaderPermutation& outShader);

	private:
		THashMap<EShaderUsage, ShaderPermutation> m_Shaders;
		THashMap<String, IMaterialParameter*> m_Parameters;
		THashMap<MaterialInstance*, TWeak<MaterialInstance>> m_MaterialInstances;
		uint64 m_Usage;
		String m_MaterialCode;

		// Each bit corresponds to a texture slot,
		// LSB = 0, MSB = 31 (0 - free, 1 - used)
		uint32 m_UsedTextureSlotsMask;

		/**
		 * @brief Is true when the Material code has been changed
		 * and the shaders need to be recompiled
		 */
		bool m_bInvalid;

		//friend TFuncMessageOnDispatch;

		friend class MaterialInstance;
	};

	// MaterialInstance Class -------------------------------------------------------------------

	class ION_API MaterialInstance
	{
	public:
		static TShared<MaterialInstance> Create(const TShared<Material>& parentMaterial);

		IMaterialParameterInstance* GetMaterialParameterInstance(const String& name) const;

		template<typename T>
		T* GetMaterialParameterInstanceTyped(const String& name) const;

		template<typename T>
		bool SetParameterValue(const String& name, const T& value) const;

		template<typename T>
		bool GetParameterValue(const String& name, T& outValue) const;

	private:
		MaterialInstance(const TShared<Material>& parentMaterial);

		void CreateParameterInstances();

	private:
		TShared<Material> m_ParentMaterial;

		THashMap<String, IMaterialParameterInstance*> m_ParameterInstances;
	};

	template<typename T>
	inline T* MaterialInstance::GetMaterialParameterInstanceTyped(const String& name) const
	{
		ionassert(dynamic_cast<T*>(GetMaterialParameterInstance(name)));
		return (T*)GetMaterialParameterInstance(name);
	}

	template<typename T>
	inline bool MaterialInstance::SetParameterValue(const String& name, const T& value) const
	{
		IMaterialParameter* parameter = GetMaterialParameterInstance(name);
		if (!parameter)
			return false;

		if constexpr (std::is_arithmetic_v<T>)
		{
			ionassert(parameter->GetType() == EMaterialParameterType::Scalar);
			MaterialParameterScalar* scalarParam = const_cast<MaterialParameterScalar*>(parameter);


		}

		if constexpr (TIsSameV<T, Vector4>)
		{
			ionassert(parameter->GetType() == EMaterialParameterType::Vector);
			MaterialParameterVector* vectorParam = const_cast<MaterialParameterVector*>(parameter);

		}

		if constexpr (TIsSameV<T, Asset>)
		{
			ionassert(value->GetType() == EAssetType::Image);
			ionassert(parameter->GetType() == EMaterialParameterType::Texture2D);

			
		}

		return true;
	}

	template<typename T>
	inline bool MaterialInstance::GetParameterValue(const String& name, T& outValue) const
	{
		return false;
	}
}

template<>
NODISCARD FORCEINLINE String ToString(Ion::EShaderUsage value)
{
	switch (value)
	{
	case Ion::EShaderUsage::None:         return "None";
	case Ion::EShaderUsage::StaticMesh:   return "StaticMesh";
	case Ion::EShaderUsage::SkeletalMesh: return "SkeletalMesh";
	case Ion::EShaderUsage::PostProcess:  return "PostProcess";
	}
	ionassert(0, "Invalid enum value.");
	return "";
}
