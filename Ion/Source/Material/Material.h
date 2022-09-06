#pragma once

#include "MaterialCommon.h"
#include "Asset/Asset.h"

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
		TRef<RHIShader> Shader;
		EShaderUsage Usage;
		bool bCompiled;

		ShaderPermutation(EShaderUsage usage);
	};

#pragma region Material Parameter

	class IMaterialParameter
	{
	public:
		virtual EMaterialParameterType GetType() const = 0;
		virtual const String& GetName() const = 0;

		virtual ~IMaterialParameter() { }

		static TMaterialParameterTypeVariant ParseParamValue(const String& val, EMaterialParameterType type, class AssetParser& parser);

	private:
		void SetValues(const TMaterialParameterTypeVariant& def, const TMaterialParameterTypeVariant& min, const TMaterialParameterTypeVariant& max);

		friend class Material;
	};

	// MaterialParameter Implementations -------------------------------------------------------

	class MaterialParameterScalar : public IMaterialParameter
	{
	public:
		MaterialParameterScalar(
			const String& name,
			float defaultValue = 0.0f,
			float min = TNumericLimits<float>::lowest(),
			float max = TNumericLimits<float>::max()
		);

		virtual EMaterialParameterType GetType() const override;
		virtual const String& GetName() const override;

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

		String m_Name;

		friend class Material;
		friend class IMaterialParameter;
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
			const String& name,
			Vector4 defaultValue = Vector4(0.0f),
			Vector4 min = Vector4(TNumericLimits<float>::lowest()),
			Vector4 max = Vector4(TNumericLimits<float>::max())
		);

		virtual EMaterialParameterType GetType() const override;
		virtual const String& GetName() const override;

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

		String m_Name;

		friend class Material;
		friend class IMaterialParameter;
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
		MaterialParameterTexture2D(
			const String& name,
			uint32 slot = 0, 
			Asset defaultValue = Asset()
		);

		virtual EMaterialParameterType GetType() const override;
		virtual const String& GetName() const override;

		virtual ~MaterialParameterTexture2D() override { }

		uint32 GetSlot() const;

		Asset GetDefaultValue() const;

	private:
		void SetDefaultValue(Asset value);

	private:
		Asset m_DefaultValue;
		uint32 m_TextureSlot;

		String m_Name;

		friend class Material;
		friend class IMaterialParameter;
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

#pragma endregion

#pragma region Shaders Compiled Counter

	using FOnShadersCompiled = TFunction<void(const ShaderPermutation&)>;

	class ShadersCompiledCounter
	{
	public:
		ShadersCompiledCounter(uint32 max);

		uint32 Inc();
		bool Done() const;

	public:
		const uint32 m_Max;
		uint32 m_Count;
	};

	inline ShadersCompiledCounter::ShadersCompiledCounter(uint32 max) :
		m_Max(max),
		m_Count(0)
	{
	}

	inline uint32 ShadersCompiledCounter::Inc()
	{
		ionassert(m_Count < m_Max, "Counter has been incremented more than the maximum value.");
		return ++m_Count;
	}

	inline bool ShadersCompiledCounter::Done() const
	{
		return m_Count == m_Max;
	}

#pragma endregion

#pragma region Material Asset Type

	class MaterialAssetType : public AssetType
	{
	public:
		FORCEINLINE explicit MaterialAssetType() :
			AssetType("Ion.Material")
		{
		}

		virtual Result<TSharedPtr<IAssetCustomData>, IOError> Parse(const std::shared_ptr<XMLDocument>& xml) override;
	};

	REGISTER_ASSET_TYPE_CLASS(MaterialAssetType);

	class MaterialAssetData : public IAssetCustomData
	{
	public:
		struct Parameter
		{
			MaterialAssetParameterValues Values;
			String Name;
			EMaterialParameterType Type;
		};

		virtual AssetType& GetType() const override
		{
			return AT_MaterialAssetType;
		}

		FilePath MaterialShaderCodePath;
		TArray<Parameter> Parameters;
	};

#pragma endregion

#pragma region Material

	class ION_API Material : public std::enable_shared_from_this<Material>
	{
	public:
		static std::shared_ptr<Material> Create();
		static std::shared_ptr<Material> CreateFromAsset(Asset materialAsset);

		/**
		 * @brief Compile the shaders for this material.
		 * 
		 * Async function - the shaders will be compiled on the worker threads.
		 */
		void CompileShaders();
		void CompileShaders(const FOnShadersCompiled& onCompiled);
		bool IsCompiled(EShaderUsage usage) const;

		bool BindShader(EShaderUsage usage) const;
		const TRef<RHIShader>& GetShader(EShaderUsage usage) const;

		void UpdateConstantBuffer() const;

		void AddUsage(EShaderUsage usage);
		void RemoveUsage(EShaderUsage usage);
		bool IsUsableWith(EShaderUsage usage);

		void SetMaterialCode(const String& code);

		void Invalidate();

		bool CompileMaterialCode();
		bool CompileMaterialCode(EShaderUsage usage);

		Asset GetAsset() const;

		~Material();

	private:
		Material();
		Material(Asset materialAsset);

		bool ParseMaterialCode(const String& code);
		bool LoadExternalMaterialCode(const FilePath& path);

		IMaterialParameter* AddParameter(const String& name, EMaterialParameterType type);
		bool RemoveParameter(const String& name);

		void DestroyParameters();

		void BuildConstantBuffer();

		uint32 GetTextureParameterCount() const;
		uint32 GetFirstFreeTextureSlot() const;

		bool CompileMaterialCode_Internal(EShaderUsage usage, ShaderPermutation& outShader);

	private:
		THashMap<EShaderUsage, ShaderPermutation> m_Shaders;
		THashMap<String, IMaterialParameter*> m_Parameters;
		THashMap<MaterialInstance*, std::weak_ptr<MaterialInstance>> m_MaterialInstances;
		TRef<RHIUniformBufferDynamic> m_MaterialConstants;
		uint64 m_Usage;
		String m_MaterialCode;
		FilePath m_MaterialShaderPath;

		Asset m_Asset;

		// Each bit corresponds to a texture slot,
		// LSB = 0, MSB = 31 (0 - free, 1 - used)
		uint32 m_UsedTextureSlotsMask;

		/**
		 * @brief Is true when the Material code has been changed
		 * and the shaders need to be recompiled
		 */
		bool m_bInvalid;

		friend class MaterialInstance;
		friend class Renderer;
	};

	inline Asset Material::GetAsset() const
	{
		return m_Asset;
	}

#pragma endregion
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
