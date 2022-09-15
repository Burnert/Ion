#pragma once

#include "Core.h"

namespace Ion
{
	REGISTER_LOGGER(MReflectionLogger, "Matter::Reflection");

#pragma region Forward Decl

	class MType;
	class MField;
	class MMethod;
	class MClass;
	class MReflection;
	class MObject;

#pragma endregion

#pragma region Matter Reflection Type class

	struct MTypeInitializer
	{
		String Name;
		size_t HashCode;
	};

	class MType
	{
	public:
		const String& GetName() const;
		size_t GetHashCode() const;

		template<typename T>
		bool Is() const;
		bool Is(MType* mType) const;

	protected:
		MType(const MTypeInitializer& initializer);

	private:
		String m_Name;
		size_t m_HashCode;

		friend class MReflection;
	};

	FORCEINLINE const String& MType::GetName() const
	{
		return m_Name;
	}

	FORCEINLINE size_t MType::GetHashCode() const
	{
		return m_HashCode;
	}

	template<typename T>
	FORCEINLINE bool MType::Is() const
	{
		return typeid(T).hash_code() == m_HashCode;
	}

	FORCEINLINE bool MType::Is(MType* mType) const
	{
		return m_HashCode == mType->m_HashCode;
	}

#pragma endregion

#pragma region Matter Reflection Field class

	namespace EFieldFlags
	{
		enum Type : uint32
		{
			None      = 0,
			Public    = 1 << 0,
			Protected = 1 << 1,
			Private   = 1 << 2,
			Static    = 1 << 3,
		};
		using UType = std::underlying_type_t<Type>;
	}

	struct MFieldInitializer
	{
		MClass* Class;
		MType* FieldType;
		String Name;
		size_t FieldOffset;
		union
		{
			EFieldFlags::UType Flags;
			struct
			{
				EFieldFlags::UType bPublic : 1;
				EFieldFlags::UType bProtected : 1;
				EFieldFlags::UType bPrivate : 1;
				EFieldFlags::UType bStatic : 1;
			};
		};
	};

	class MField
	{
	public:
		const String& GetName() const;

		const MClass* GetClass() const;

		const MType* GetType() const;

	private:
		MField(const MFieldInitializer& initializer);

	private:
		MClass* m_Class;

		MType* m_FieldType;

		String m_Name;

		union
		{
			EFieldFlags::UType m_Flags;
			struct
			{
				EFieldFlags::UType m_bPublic : 1;
				EFieldFlags::UType m_bProtected : 1;
				EFieldFlags::UType m_bPrivate : 1;
				EFieldFlags::UType m_bStatic : 1;
			};
		};

		friend class MReflection;
	};

	FORCEINLINE const String& MField::GetName() const
	{
		return m_Name;
	}
	
	FORCEINLINE const MClass* MField::GetClass() const
	{
		return m_Class;
	}

	FORCEINLINE const MType* MField::GetType() const
	{
		return m_FieldType;
	}

#pragma endregion

#pragma region Matter Reflection Method class

	namespace EMethodFlags
	{
		enum Type : uint32
		{
			None      = 0,
			Public    = 1 << 0,
			Protected = 1 << 1,
			Private   = 1 << 2,
			Static    = 1 << 3,
			Virtual   = 1 << 4,
		};
		using UType = std::underlying_type_t<Type>;
	}

	struct MMethodInitializer
	{
		MClass* Class;
		MType* ReturnType;
		TArray<MType*> ParameterTypes;
		String Name;
		union
		{
			EMethodFlags::UType Flags;
			struct
			{
				EMethodFlags::UType bPublic : 1;
				EMethodFlags::UType bProtected : 1;
				EMethodFlags::UType bPrivate : 1;
				EMethodFlags::UType bStatic : 1;
				EMethodFlags::UType bVirtual : 1;
			};
		};
	};

	class MMethod
	{
	public:
		template<typename... Args>
		void Invoke(Args&&... args);

		const String& GetName() const;

		MClass* GetClass() const;

		MType* GetReturnType() const;
		const TArray<MType*>& GetParameterTypes() const;

	private:
		MMethod(const MMethodInitializer& initializer);

	private:
		MClass* m_Class;

		MType* m_ReturnType;
		TArray<MType*> m_ParameterTypes;

		String m_Name;

		union
		{
			EMethodFlags::UType m_Flags;
			struct
			{
				EMethodFlags::UType m_bPublic : 1;
				EMethodFlags::UType m_bProtected : 1;
				EMethodFlags::UType m_bPrivate : 1;
				EMethodFlags::UType m_bStatic : 1;
				EMethodFlags::UType m_bVirtual : 1;
			};
		};

		friend class MReflection;
	};

	FORCEINLINE const String& MMethod::GetName() const
	{
		return m_Name;
	}

	FORCEINLINE MClass* MMethod::GetClass() const
	{
		return m_Class;
	}

	FORCEINLINE MType* MMethod::GetReturnType() const
	{
		return m_ReturnType;
	}

	FORCEINLINE const TArray<MType*>& MMethod::GetParameterTypes() const
	{
		return m_ParameterTypes;
	}

#pragma endregion

#pragma region Matter Reflection Class class

	using FMClassInstantiateDefault = TFunction<MObject* (const MObject*)>;

	struct MClassInitializer
	{
		String Name;
		size_t TypeHashCode;
		MObject* CDO;
		MClass* SuperClass;
		FMClassInstantiateDefault InstantiateFunc;
	};

	class MClass : public MType
	{
	public:
		MObject* Instantiate() const;

		const MObject* GetClassDefaultObject() const;

		template<typename T>
		FORCEINLINE const T& GetClassDefaultObjectTyped() const
		{
			ionassert(m_CDO->GetClass()->GetHashCode() == typeid(T).hash_code());
			return *reinterpret_cast<T*>(m_CDO);
		}

		MClass* GetSuperClass() const;

		const TArray<MField*>& GetFields() const;
		const TArray<MMethod*>& GetMethods() const;

	private:
		MClass(const MClassInitializer& initializer);

		void SetupClassDefaultObject(const String& name);

	private:
		MClass* m_SuperClass;
		TArray<MField*> m_Fields;
		TArray<MMethod*> m_Methods;

		MObject* m_CDO;
		FMClassInstantiateDefault m_FInstantiate;

		friend class MReflection;
		friend class MObject;

	public:
		friend Archive& operator<<(Archive& ar, MClass*& mClass);
	};

	FORCEINLINE const MObject* MClass::GetClassDefaultObject() const
	{
		return m_CDO;
	}

	FORCEINLINE MClass* MClass::GetSuperClass() const
	{
		return m_SuperClass;
	}

	FORCEINLINE const TArray<MField*>& MClass::GetFields() const
	{
		return m_Fields;
	}

	FORCEINLINE const TArray<MMethod*>& MClass::GetMethods() const
	{
		return m_Methods;
	}

#pragma endregion

#pragma region Templates

	// TIsReflectableClass --------------------------------------------------------------------

	template<typename T, typename = void>
	struct TIsReflectableClass { static constexpr bool Value = false; };

	template<typename T>
	struct TIsReflectableClass<T, std::void_t<decltype(T::StaticClass())>> { static constexpr bool Value = true; };

	template<typename T>
	static inline constexpr bool TIsReflectableClassV = TIsReflectableClass<T>::Value;

	// TGetReflectableType -------------------------------------------------------------------

	template<typename T, typename = void>
	struct TGetReflectableType { static MType* Type() { return nullptr; } };

	template<typename T>
	struct TGetReflectableType<T, TEnableIfT<TIsReflectableClassV<TRemovePtr<T>>>>
	{
		static MType* Type() { return TRemovePtr<T>::StaticClass(); }
	};

	// TGetReflectableClass -------------------------------------------------------------------

	template<typename T, typename = void>
	struct TGetReflectableClass { static MClass* Class() { return nullptr; } };

	template<typename T>
	struct TGetReflectableClass<T, TEnableIfT<TIsReflectableClassV<TRemovePtr<T>>>>
	{
		static MClass* Class() { return static_cast<MClass*>(TGetReflectableType<T>::Type()); }
	};

	// TGetSuperClass ------------------------------------------------------------------------

	template<typename T, typename = void>
	struct TGetSuperClass { static MClass* Class() { return nullptr; } };

	template<typename T>
	struct TGetSuperClass<T, std::void_t<typename T::Super>> { static MClass* Class() { return T::Super::StaticClass(); } };

	// TMethodParamPack ---------------------------------------------------------------------

	template<typename... Args>
	struct TMethodParamPack { };

	template<>
	struct TMethodParamPack<> { };

	template<typename TFirst, typename... TRest>
	struct TMethodParamPack<TFirst, TRest...> { using Type = TFirst; using RestTypes = TMethodParamPack<TRest...>; };

	template<typename TLast>
	struct TMethodParamPack<TLast> { using Type = TLast; using RestTypes = TMethodParamPack<>; };

	// TMethodGetReturnType ---------------------------------------------------------------------

	template<typename TClass, typename FMethod, typename TParamPack>
	struct TMethodGetReturnType { };

	template<typename TClass, typename FMethod, typename... TParams>
	struct TMethodGetReturnType<TClass, FMethod, TMethodParamPack<TParams...>>
	{
		using Type = std::invoke_result_t<FMethod, TClass, TParams...>;
	};

	// TMethodGetReflectableParamTypes -------------------------------------------------------------

	template<typename TParamPack>
	struct TMethodGetReflectableParamTypes { };

	template<typename... TParams>
	struct TMethodGetReflectableParamTypes<TMethodParamPack<TParams...>>
	{
		static TArray<MType*> Params() { return TArray<MType*> { TGetReflectableType<TParams>::Type()... }; }
	};

#pragma endregion

#pragma region Matter Reflection Registry

// Damn you Windows
#undef RegisterClass

	class ION_API MReflection
	{
	public:
		static MType* RegisterType(const MTypeInitializer& initializer);
		static MClass* RegisterClass(const MClassInitializer& initializer);
		static MField* RegisterField(const MFieldInitializer& initializer);
		static MMethod* RegisterMethod(const MMethodInitializer& initializer);

		static MClass* FindClassByName(const String& name);
		static MType* FindTypeByName(const String& name);

	private:
		static inline TArray<MType*> s_ReflectableTypeRegistry;
		static inline TArray<MClass*> s_MClassRegistry;
	};

#pragma endregion

#pragma region Reflection Macros

#define MTYPE(T) \
inline MType* const MatterRT_##T = [] { \
	static MTypeInitializer c_Initializer { \
		/* MType name (same as MClass if used as a base) */ \
		#T, \
		/* The compiler provided type hash code */ \
		typeid(T).hash_code() \
	}; \
	return MReflection::RegisterType(c_Initializer); \
}(); \
template<> struct TGetReflectableType<T> { static MType* Type() { return MatterRT_##T; } };

#define MCLASS(T) \
public: \
using TThisClass = T; \
FORCEINLINE static MClass* StaticClass() { \
	static MClassInitializer c_Initializer { \
		/* Default MObject name (the MClass name is based on it) */ \
		#T, \
		/* The compiler provided type hash code */ \
		typeid(T).hash_code(), \
		/* CDO instance */ \
		new T, \
		/* The super class (nullptr if MObject as it doesn't inherit from anything) */ \
		TGetSuperClass<T>::Class(), \
		/* Instantiate function - Copy construct a new object from the CDO instance. */ \
		[](const MObject* cdo) { \
			return new T(*static_cast<const T*>(cdo)); \
		} \
	}; \
	static MClass* c_Class = MReflection::RegisterClass(c_Initializer); \
	return c_Class; \
} \
static inline MClass* const MatterRT = StaticClass();

#define MFIELD(name) \
static inline MField* MatterRF_##name = [] { \
	using TField = decltype(name); \
	static MType* fieldType = TGetReflectableType<TField>::Type(); \
	static MFieldInitializer c_Initializer { \
		/* The owning class */ \
		StaticClass(), \
		/* Reflectable field type */ \
		fieldType, \
		/* The name of the field */ \
		#name, \
		/* The offset of the actual field */ \
		/* @TODO: Doesn't work, figure something out... (code parsing. eh) */ \
		0, /*offsetof(TThisClass, name),*/ \
		/* Attributes (accessibility, static, etc.) */ \
		EFieldFlags::None  /* @TODO: I don't think there's a way to get the visibility of a field without code parsing. */ \
	}; \
	return MReflection::RegisterField(c_Initializer); \
}();

#define MMETHOD(name, ...) \
static inline MMethod* MatterRM_##name = [] { \
	using FMethod = decltype(&TThisClass::name); \
	using TMethodParamTypes = TMethodParamPack<__VA_ARGS__>; \
	using TReturn = typename TMethodGetReturnType<TThisClass, FMethod, TMethodParamTypes>::Type; \
	static TArray<MType*> parameterTypes = TMethodGetReflectableParamTypes<TMethodParamTypes>::Params(); \
	static MMethodInitializer c_Initializer { \
		/* The owning class */ \
		StaticClass(), \
		/* Method return type */ \
		TGetReflectableType<TReturn>::Type(), \
		/* Method parameter types */ \
		parameterTypes, \
		/* The name of the method */ \
		#name, \
		/* Attributes (accessibility, static, etc.) */ \
		EMethodFlags::None \
	}; \
	return MReflection::RegisterMethod(c_Initializer); \
}();

#pragma endregion

#pragma region Fundamental Reflectable Types

	MTYPE(int8)
	MTYPE(int16)
	MTYPE(int32)
	MTYPE(int64)
	MTYPE(uint8)
	MTYPE(uint16)
	MTYPE(uint32)
	MTYPE(uint64)
	MTYPE(bool)
	MTYPE(float)
	MTYPE(double)

	// @TODO: Pointers? Might require wrappers

	// @TODO: Collections:
	// - Dynamic Array
	// - Fixed Array
	// - Hash Set
	// - Hash Map

#pragma endregion
}
