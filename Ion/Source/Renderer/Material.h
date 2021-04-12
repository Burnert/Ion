#pragma once

namespace Ion
{
	class Shader;

	enum class EMaterialPropertyType : ubyte
	{
		Float,
		Float2,
		Float3,
		Float4,
		Bool,
	};

	inline static constexpr const char* MaterialPropertyTypeToString(EMaterialPropertyType type)
	{
		switch (type)
		{
		case EMaterialPropertyType::Float:  return TypeToString<float>();
		case EMaterialPropertyType::Float2: return TypeToString<FVector2>();
		case EMaterialPropertyType::Float3: return TypeToString<FVector3>();
		case EMaterialPropertyType::Float4: return TypeToString<FVector4>();
		case EMaterialPropertyType::Bool:   return TypeToString<bool>();
		default:                            return TypeToString<void>();
		}
	}

	// Base Material Property class
	template<typename T, typename EnableT = void>
	class MaterialProperty { };

	// Partial Material Property specialization for float types
	template<typename T>
	class MaterialProperty<T,
		typename TEnableIfT<TOrV<
			TIsSame<T, float>,
			TIsSame<T, FVector2>,
			TIsSame<T, FVector3>,
			TIsSame<T, FVector4>
		>>
	>
	{
		friend class Material;
	public:
		MaterialProperty() :
			m_Value(0.0f),
			m_Min(-1.0f),
			m_Max(1.0f)
		{
			if constexpr      (TIsSameV<T, float>)    m_Type = EMaterialPropertyType::Float;
			else if constexpr (TIsSameV<T, FVector2>) m_Type = EMaterialPropertyType::Float2;
			else if constexpr (TIsSameV<T, FVector3>) m_Type = EMaterialPropertyType::Float3;
			else if constexpr (TIsSameV<T, FVector4>) m_Type = EMaterialPropertyType::Float4;
		}

		FORCEINLINE void SetValue(T value)
		{
			m_Value = glm::clamp(value, m_Min, m_Max);
		}

		FORCEINLINE const T& GetValue() const
		{
			return m_Value;
		}

		FORCEINLINE void SetMax(T max)
		{
			m_Max = max;
			m_Value = glm::min(m_Value, m_Max);
		}

		FORCEINLINE const T& GetMax() const
		{
			return m_Max;
		}

		FORCEINLINE void SetMin(T min)
		{
			m_Min = min;
			m_Value = glm::max(m_Value, m_Min);
		}

		FORCEINLINE const T& GetMin() const
		{
			return m_Min;
		}

	private:
		EMaterialPropertyType m_Type;
		T m_Value;
		T m_Min;
		T m_Max;
	};

	// Bool Material Property specialization
	template<typename T>
	class MaterialProperty<T,
		typename TEnableIfT<TIsSameV<T, bool>>
	>
	{
		friend class Material;
	public:
		MaterialProperty() :
			m_Value(false),
			m_Type(EMaterialPropertyType::Bool)
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
		EMaterialPropertyType m_Type;
		bool m_Value;
	};

	class ION_API Material
	{
		friend class Renderer;
		friend class OpenGLRenderer;
	public:
		static TShared<Material> Create();

		~Material();

		void SetShader(const TShared<Shader>& shader);
		FORCEINLINE const TShared<Shader>& GetShader() const { return m_Shader; }

		FORCEINLINE bool HasMaterialProperty(const String& name) const
		{
			return m_Properties.find(name) != m_Properties.end();
		}

		void CreateMaterialProperty(const String& name, EMaterialPropertyType type);

		template<typename T>
		void SetMaterialProperty(const String& name, const T& value)
		{
			MaterialProperty<T>* prop = FindProperty<T>(name);
			if (!prop)
			{
				LOG_WARN("Cannot set property '{0}<{1}>' because it does not exist!", name, TypeToString<T>());
				return;
			}
			prop->SetValue(value);
		}

		template<typename T>
		MaterialProperty<T>* GetMaterialProperty(const String& name)
		{
			MaterialProperty<T>* prop = FindProperty<T>(name);
			if (!prop)
			{
				LOG_WARN("Property '{0}<{1}>' does not exist!", name, TypeToString<T>());
			}
			return prop;
		}

		void RemoveMaterialProperty(const String& name);

		void LinkPropertyToUniform(const String& name, const String& uniformName);

		void UpdateShaderUniforms();

		template<typename Type>
		inline static constexpr bool IsStaticTypeSame(EMaterialPropertyType type)
		{
			return
				TIsSameV<Type, float>    && type == EMaterialPropertyType::Float  ||
				TIsSameV<Type, FVector2> && type == EMaterialPropertyType::Float2 ||
				TIsSameV<Type, FVector3> && type == EMaterialPropertyType::Float3 ||
				TIsSameV<Type, FVector4> && type == EMaterialPropertyType::Float4 ||
				TIsSameV<Type, bool>     && type == EMaterialPropertyType::Bool;
		}

	protected:
		Material();

		template<typename T>
		MaterialProperty<T>* FindProperty(const String& name) const
		{
			const auto& it = m_Properties.find(name);
			if (it == m_Properties.end())
			{
				return nullptr;
			}
			// Get the property type from void* and check it with the template type.
			// If this check is here, an incorrect reinterpret cast cannot occur.
			EMaterialPropertyType type = ExtractPropertyType(it->second);
			if (!IsStaticTypeSame<T>(type))
			{
				LOG_ERROR("Tried to cast the property '{0}' to incorrect value type! ({1} instead of {2})",
					name, TypeToString<T>(), MaterialPropertyTypeToString(type));
				return nullptr;
			}
			MaterialProperty<T>* prop = reinterpret_cast<MaterialProperty<T>*>(it->second);
			return prop;
		}

		inline static EMaterialPropertyType ExtractPropertyType(void* propertyPtr)
		{
			// The first field of the object is always the type, so if the pointer points
			// to a valid MaterialProperty object this cast must yield a correct value.
			return *(EMaterialPropertyType*)propertyPtr;
		}

		//FORCEINLINE const std::unordered_map<String, void*>& GetMaterialProperties() const { return m_Properties; }
		//FORCEINLINE const std::unordered_map<String, String>& GetUniformLinks() const { return m_UniformLinks; }

	private:
		TShared<Shader> m_Shader;
		std::unordered_map<String, void*> m_Properties;
		std::unordered_map<String, String> m_UniformLinks;

		// @TODO: Add texture pointers to material
	};
}

