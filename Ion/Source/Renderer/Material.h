#pragma once

namespace Ion
{
	enum class EMaterialParameterType : uint8
	{
		Float,
		Float2,
		Float3,
		Float4,
		Bool,
		Texture2D,
	};

	inline static constexpr const char* MaterialParameterTypeToString(EMaterialParameterType type)
	{
		switch (type)
		{
		case EMaterialParameterType::Float:      return TypeToString<float>();
		case EMaterialParameterType::Float2:     return TypeToString<Vector2>();
		case EMaterialParameterType::Float3:     return TypeToString<Vector3>();
		case EMaterialParameterType::Float4:     return TypeToString<Vector4>();
		case EMaterialParameterType::Bool:       return TypeToString<bool>();
		case EMaterialParameterType::Texture2D:  return "Texture2D";
		default:                                 return TypeToString<void>();
		}
	}

	// Base Material Parameter class
	template<typename T, typename EnableT = void>
	class TMaterialParameter { };

	// Partial Material Parameter specialization for float types
	template<typename T>
	class TMaterialParameter<T,
		typename TEnableIfT<TOrV<
			TIsSame<T, float>,
			TIsSame<T, Vector2>,
			TIsSame<T, Vector3>,
			TIsSame<T, Vector4>
		>>
	>
	{
		friend class Material;
	public:
		TMaterialParameter() :
			m_Type(EMaterialParameterType::Float),
			m_Value(0.0f),
			m_Min(-1.0f),
			m_Max(1.0f)
		{
			if constexpr      (TIsSameV<T, float>)   m_Type = EMaterialParameterType::Float;
			else if constexpr (TIsSameV<T, Vector2>) m_Type = EMaterialParameterType::Float2;
			else if constexpr (TIsSameV<T, Vector3>) m_Type = EMaterialParameterType::Float3;
			else if constexpr (TIsSameV<T, Vector4>) m_Type = EMaterialParameterType::Float4;
		}

		FORCEINLINE void SetValue(T value)
		{
			m_Value = Math::Clamp(value, m_Min, m_Max);
		}

		FORCEINLINE const T& GetValue() const
		{
			return m_Value;
		}

		FORCEINLINE void SetMax(T max)
		{
			m_Max = max;
			m_Value = Math::Min(m_Value, m_Max);
		}

		FORCEINLINE const T& GetMax() const
		{
			return m_Max;
		}

		FORCEINLINE void SetMin(T min)
		{
			m_Min = min;
			m_Value = Math::Max(m_Value, m_Min);
		}

		FORCEINLINE const T& GetMin() const
		{
			return m_Min;
		}

	private:
		EMaterialParameterType m_Type;
		T m_Value;
		T m_Min;
		T m_Max;
	};

	// Bool Material Parameter specialization
	template<typename T>
	class TMaterialParameter<T,
		typename TEnableIfT<TIsSameV<T, bool>>
	>
	{
		friend class Material;
	public:
		TMaterialParameter() :
			m_Value(false),
			m_Type(EMaterialParameterType::Bool)
		{ }

		FORCEINLINE void SetValue(bool value)
		{
			m_Value = value;
		}

		FORCEINLINE bool GetValue() const
		{
			return m_Value;
		}

	private:
		EMaterialParameterType m_Type;
		bool m_Value;
	};

	class RHITexture;

	// Texture2D Material Parameter specialization
	template<typename T>
	class TMaterialParameter<T,
		// @TODO: This TShared<Texture> is weird but necessary with this kind of implementation
		typename TEnableIfT<TIsSameV<T, TShared<RHITexture>>>>
	{
		friend class Material;
	public:
		TMaterialParameter(uint32 slot) :
			m_Type(EMaterialParameterType::Texture2D),
			m_ReservedSlot(slot)
		{ }

		FORCEINLINE void SetValue(const TShared<RHITexture>& texture)
		{
			m_Texture = texture;
		}

		FORCEINLINE const TWeak<RHITexture>& GetValue() const
		{
			return m_Texture;
		}

	private:
		EMaterialParameterType m_Type;
		TWeak<RHITexture> m_Texture;
		uint32 m_ReservedSlot;
	};

	class RHIShader;

	class ION_API Material
	{
		friend class Renderer;
		friend class Scene;
#if PLATFORM_SUPPORTS_OPENGL
		friend class OpenGLRenderer;
#endif

	public:
		static TShared<Material> Create();

		~Material();

		void SetShader(const TShared<RHIShader>& shader);
		FORCEINLINE const TShared<RHIShader>& GetShader() const { return m_Shader; }

		FORCEINLINE bool HasParameter(const String& name) const
		{
			return m_Parameters.find(name) != m_Parameters.end();
		}

		void CreateParameter(const String& name, EMaterialParameterType type);

		template<typename T>
		void SetParameter(const String& name, const T& value)
		{
			TMaterialParameter<T>* param = FindParameter<T>(name);
			if (!param)
			{
				LOG_WARN("Cannot set parameter '{0}<{1}>' because it does not exist!", name, TypeToString<T>());
				return;
			}
			param->SetValue(value);
		}

		template<typename T>
		TMaterialParameter<T>* GetParameter(const String& name)
		{
			TMaterialParameter<T>* param = FindParameter<T>(name);
			if (!param)
			{
				LOG_WARN("Parameter '{0}<{1}>' does not exist!", name, TypeToString<T>());
			}
			return param;
		}

		void RemoveParameter(const String& name);

		void LinkParameterToUniform(const String& name, const String& uniformName);

		void UpdateShaderUniforms() const;

		void BindTextures() const;

		template<typename Type>
		inline static constexpr bool IsStaticTypeSame(EMaterialParameterType type)
		{
			return
				TIsSameV<Type, float>    && type == EMaterialParameterType::Float  ||
				TIsSameV<Type, Vector2>  && type == EMaterialParameterType::Float2 ||
				TIsSameV<Type, Vector3>  && type == EMaterialParameterType::Float3 ||
				TIsSameV<Type, Vector4>  && type == EMaterialParameterType::Float4 ||
				TIsSameV<Type, bool>     && type == EMaterialParameterType::Bool   ||
				TIsSameV<Type, TShared<RHITexture>>  && type == EMaterialParameterType::Texture2D;
		}

		template<typename Lambda>
		void ForEachTexture(Lambda func) const
		{
			for (TMaterialParameter<TShared<RHITexture>>* const& textureParam : m_TextureParameters)
			{
				func(textureParam->m_Texture.lock(), textureParam->m_ReservedSlot);
			}
		}

	protected:
		Material();

		template<typename T>
		TMaterialParameter<T>* FindParameter(const String& name) const
		{
			const auto& it = m_Parameters.find(name);
			if (it == m_Parameters.end())
			{
				return nullptr;
			}
			// Get the parameter type from void* and check it with the template type.
			// If this check is here, an incorrect reinterpret cast cannot occur.
			EMaterialParameterType type = ExtractParameterType(it->second);
			if (!IsStaticTypeSame<T>(type))
			{
				LOG_ERROR("Tried to cast the parameter '{0}' to incorrect value type! ({1} instead of {2})",
					name, TypeToString<T>(), MaterialParameterTypeToString(type));
				return nullptr;
			}
			TMaterialParameter<T>* param = reinterpret_cast<TMaterialParameter<T>*>(it->second);
			return param;
		}

		inline static EMaterialParameterType ExtractParameterType(void* paramPtr)
		{
			ionassert(IsAnyOf(*(EMaterialParameterType*)paramPtr,
				EMaterialParameterType::Bool,
				EMaterialParameterType::Float,
				EMaterialParameterType::Float2,
				EMaterialParameterType::Float3,
				EMaterialParameterType::Float4,
				EMaterialParameterType::Texture2D), "Used ExtractParameterType on a pointer of incorrect type! This will lead to unexpected behavior.");
			// The first field of the object is always the type, so if the pointer points
			// to a valid TMaterialParameter object this cast must yield a correct value.
			return *(EMaterialParameterType*)paramPtr;
		}

		FORCEINLINE const RHIShader* GetShaderRaw() const { return m_Shader.get(); }

	private:
		// @TODO: Make it so that each type has its own collection (should be faster this way)

		TShared<RHIShader> m_Shader;
		THashMap<String, void*> m_Parameters;
		THashMap<String, String> m_UniformLinks;
		THashSet<TMaterialParameter<TShared<RHITexture>>*> m_TextureParameters;
	};
}

