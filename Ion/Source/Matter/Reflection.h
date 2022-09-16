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

	namespace ETypeFlags
	{
		enum Type
		{
			None        = 0,
			Fundamental = 1 << 0,
			Class       = 1 << 1,
		};
		using UType = std::underlying_type_t<Type>;
	}

	struct MTypeInitializer
	{
		String Name;
		size_t HashCode;
		size_t Size;
		union
		{
			ETypeFlags::UType Flags;
			struct
			{
				ETypeFlags::UType bFundamental : 1;
				ETypeFlags::UType bClass : 1;
			};
		};
	};

	class MType
	{
	public:
		const String& GetName() const;
		size_t GetHashCode() const;
		size_t GetSize() const;

		template<typename T>
		bool Is() const;
		bool Is(MType* mType) const;

		bool IsFundamental() const;
		bool IsClass() const;

	protected:
		MType(const MTypeInitializer& initializer);

	private:
		String m_Name;
		size_t m_HashCode;
		size_t m_Size;
		union
		{
			ETypeFlags::UType m_Flags;
			struct
			{
				ETypeFlags::UType m_bFundamental : 1;
				ETypeFlags::UType m_bClass : 1;
			};
		};

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

	FORCEINLINE size_t MType::GetSize() const
	{
		return m_Size;
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

	FORCEINLINE bool MType::IsFundamental() const
	{
		return m_bFundamental;
	}

	FORCEINLINE bool MType::IsClass() const
	{
		return m_bClass;
	}

#pragma endregion

#pragma region Matter Generic Value wrapper

	class MValue
	{
	public:
		template<typename T>
		static TSharedPtr<MValue> Create(const T& value);

		virtual MType* GetType() const = 0;

		template<typename T>
		const T& As() const
		{
			static_assert(TIsReflectableTypeV<T>);
			ionassert(TGetReflectableType<T>::Type()->Is(GetType()));
			return *(const T*)GetValuePointer();
		}

	protected:
		virtual const void* GetValuePointer() const = 0;
	};

	template<typename T>
	class TMValue : public MValue
	{
	public:
		FORCEINLINE TMValue(const T& value) :
			m_Value(value)
		{
			static_assert(TIsReflectableTypeV<T>);
		}

		virtual MType* GetType() const override
		{
			return TGetReflectableType<T>::Type();
		}

	protected:
		virtual const void* GetValuePointer() const override
		{
			return &m_Value;
		}

	private:
		T m_Value;
	};

	template<>
	class TMValue<void> : public MValue
	{
	public:
		virtual MType* GetType() const override
		{
			ionbreak("Called GetType on a void value.");
			return nullptr;
		}

	protected:
		virtual const void* GetValuePointer() const override
		{
			ionbreak("Called GetValuePointer on a void value.");
			return nullptr;
		}
	};

	template<typename T>
	FORCEINLINE static TSharedPtr<MValue> MValue::Create(const T& value)
	{
		return MakeShared<TMValue<T>>(value);
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
		size_t FieldOffset;
		String Name;
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

		size_t GetOffset() const;

		template<typename T>
		void SetValue(MObject* object, const T& value);

		template<typename T>
		T GetValue(MObject* object);

	private:
		MField(const MFieldInitializer& initializer);

	private:
		MClass* m_Class;

		MType* m_FieldType;
		size_t m_FieldOffset;

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

	FORCEINLINE size_t MField::GetOffset() const
	{
		return m_FieldOffset;
	}

	template<typename T>
	FORCEINLINE void MField::SetValue(MObject* object, const T& value)
	{
		ionassert(object);
		ionassert(m_FieldType->Is(TGetReflectableType<T>::Type()), "Wrong type has been passed.");
		ionassert(!m_FieldType->IsClass() || TIsPointerV<T>);
		ionassert(m_FieldType->IsClass() || !TIsPointerV<T>);
		ionassert(m_FieldType->GetSize() == sizeof(TRemovePtr<T>));
		ionassert(m_FieldType->GetHashCode() == typeid(TRemovePtr<T>).hash_code());

		T* pValue = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(object) + m_FieldOffset);
		*pValue = value;
	}

	template<typename T>
	FORCEINLINE T MField::GetValue(MObject* object)
	{
		ionassert(object);
		ionassert(m_FieldType->Is(TGetReflectableType<T>::Type()), "Wrong type has been passed.");
		ionassert(!m_FieldType->IsClass() || TIsPointerV<T>);
		ionassert(m_FieldType->IsClass() || !TIsPointerV<T>);
		ionassert(m_FieldType->GetSize() == sizeof(TRemovePtr<T>));
		ionassert(m_FieldType->GetHashCode() == typeid(TRemovePtr<T>).hash_code());

		T* pValue = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(object) + m_FieldOffset);
		return *pValue;
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

	/*                               void(object,   parameters,                 out return value) */
	using FMMethodInvoke = TFunction<void(MObject*, TArray<TSharedPtr<MValue>>, TSharedPtr<MValue>&)>;

	struct MMethodInitializer
	{
		MClass* Class;
		MType* ReturnType;
		TArray<MType*> ParameterTypes;
		String Name;
		FMMethodInvoke FInvoke;
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
		template<typename TRet = void, typename... Args>
		TRet Invoke(MObject* object = nullptr, Args&&... args);

		TSharedPtr<MValue> InvokeEx(MObject* object = nullptr);
		TSharedPtr<MValue> InvokeEx(MObject* object, const TArray<TSharedPtr<MValue>>& params);

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

		FMMethodInvoke m_FInvoke;

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

	template<typename TRet, typename... Args>
	FORCEINLINE TRet MMethod::Invoke(MObject* object, Args&&... args)
	{
		TSharedPtr<MValue> retValue = InvokeEx(object, { MValue::Create(args)... });
		if constexpr (!std::is_void_v<TRet>)
			return retValue->As<TRet>();
	}

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
		MTypeInitializer TypeInitializer;
		String CDOName;
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
		const T& GetClassDefaultObjectTyped() const;

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

	template<typename T>
	FORCEINLINE const T& MClass::GetClassDefaultObjectTyped() const
	{
		ionassert(m_CDO->GetClass()->GetHashCode() == typeid(T).hash_code());
		return *reinterpret_cast<T*>(m_CDO);
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

	// TIsReflectableType --------------------------------------------------------------------

	template<typename T, typename = void>
	struct TIsReflectableType { static constexpr bool Value = false; };

	template<typename T>
	struct TIsReflectableType<T, TEnableIfT<TIsReflectableClassV<TRemovePtr<T>>>> { static constexpr bool Value = true; };

	template<typename T>
	static inline constexpr bool TIsReflectableTypeV = TIsReflectableType<T>::Value;

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
	struct TMethodParamPack<>
	{
		static constexpr size_t Count = 0;
	};

	template<typename TFirst, typename... TRest>
	struct TMethodParamPack<TFirst, TRest...>
	{
		static constexpr size_t Count = 1 + sizeof...(TRest);
		using Type = TFirst;
		using Rest = TMethodParamPack<TRest...>;
	};

	template<typename TLast>
	struct TMethodParamPack<TLast>
	{
		static constexpr size_t Count = 1;
		using Type = TLast;
		using Rest = TMethodParamPack<>;
	};

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

	// TGetNthParamType -------------------------------------------------------------------------

	template<size_t N, typename TParamPack>
	struct TGetNthParamType { };

	template<size_t N, typename... TParams>
	struct TGetNthParamType<N, TMethodParamPack<TParams...>>
	{
		using Type = typename TGetNthParamType<N - 1, typename TMethodParamPack<TParams...>::Rest>::Type;
	};

	template<typename... TParams>
	struct TGetNthParamType<0, TMethodParamPack<TParams...>>
	{
		using Type = typename TMethodParamPack<TParams...>::Type;
	};

	template<size_t N>
	struct TGetNthParamType<N, TMethodParamPack<>>
	{
		using Type = void;
	};

	// TExtractParamForInvoke ---------------------------------------------------------------------

	template<typename TParamPack, size_t Index>
	struct TExtractParamForInvoke { };

	template<size_t Index, typename... TParams>
	struct TExtractParamForInvoke<TMethodParamPack<TParams...>, Index>
	{
		using TParamPack = TMethodParamPack<TParams...>;
		FORCEINLINE static auto& Extract(const TArray<TSharedPtr<MValue>>& parameters)
		{
			using ParamType = typename TGetNthParamType<Index, TParamPack>::Type;

			const TSharedPtr<MValue>& value = parameters[Index];
			ionassert(Index < parameters.size());
			ionassert(value->GetType()->Is(TGetReflectableType<ParamType>::Type()),
				"Wrong parameter type has been passed at index {}. {} instead of {}",
				Index, value->GetType()->GetName(), TGetReflectableType<ParamType>::Type()->GetName());

			return value->As<ParamType>();
		}
	};

	// TMethodInvoker ----------------------------------------------------------------------------
	
	template<typename TClass, typename TFuncPtr, TFuncPtr Func, typename TReturn, typename TParamPack>
	struct TMethodInvoker
	{
		FORCEINLINE static void Invoke(
			TClass* object,
			const TArray<TSharedPtr<MValue>>& parameters,
			TSharedPtr<MValue>& outRetVal)
		{
			ionassert(object); // @TODO: Unless static
			ionassert(parameters.size() == TParamPack::Count,
				"A wrong amount of parameters has been passed to the invoke function. {} instead of {}.",
				parameters.size(), TParamPack::Count);

			Invoke_Impl(object, parameters, outRetVal, std::make_index_sequence<TParamPack::Count> {});
		}

	private:
		template<size_t... I>
		FORCEINLINE static void Invoke_Impl(
			TClass* object,
			const TArray<TSharedPtr<MValue>>& parameters,
			TSharedPtr<MValue>& outRetVal,
			std::index_sequence<I...> seq)
		{
			static_assert(sizeof...(I) == TParamPack::Count);
			ionassert(sizeof...(I) == parameters.size());

			if constexpr (!std::is_void_v<TReturn>)
			{
				outRetVal = MValue::Create<TReturn>( (object->*Func)(ExtractParam<I>(parameters)...) );
			}
			else
			{
				(object->*Func)(Extract<I>(parameters)...);
				outRetVal = nullptr;
			}
		}

		template<size_t I>
		FORCEINLINE static auto& ExtractParam(const TArray<TSharedPtr<MValue>>& parameters)
		{
			return TExtractParamForInvoke<TParamPack, I>::Extract(parameters);
		}
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
		typeid(T).hash_code(), \
		/* Type size in bytes */ \
		sizeof(T), \
		/* Type attributes */ \
		ETypeFlags::Fundamental \
	}; \
	return MReflection::RegisterType(c_Initializer); \
}(); \
template<> struct TGetReflectableType<T> { static MType* Type() { return MatterRT_##T; } }; \
template<> struct TIsReflectableType<T> { static constexpr bool Value = true; };

#define MCLASS(T) \
public: \
using TThisClass = T; \
FORCEINLINE static MClass* StaticClass() { \
	static MClassInitializer c_Initializer { \
		MTypeInitializer { \
			/* The MClass name (C_ prefix to differentiate a class name from an object name) */ \
			"C_"s + #T, \
			/* The compiler provided type hash code */ \
			typeid(T).hash_code(), \
			/* Type size in bytes */ \
			sizeof(T), \
			/* Type attributes */ \
			ETypeFlags::Class \
		}, \
		/* Default MObject name */ \
		#T, \
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
		/* Offset of the actual field (calculated using a pointer to member) */ \
		[] { return (size_t)&(((TThisClass*)nullptr)->*(&TThisClass::name)); }(), \
		/* The name of the field */ \
		#name, \
		/* Attributes (accessibility, static, etc.) */ \
		EFieldFlags::None  /* @TODO: I don't think there's a way to get the visibility of a field without code parsing. */ \
	}; \
	return MReflection::RegisterField(c_Initializer); \
}();

#define MMETHOD(name, ...) \
static inline MMethod* MatterRM_##name = [] { \
	using FMethod = decltype(&TThisClass::name); \
	using TMethodParamTypes = TMethodParamPack<__VA_ARGS__>; \
	static constexpr size_t ParamCount = TMethodParamTypes::Count; \
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
		/* MMethod FInvoke function */ \
		[](MObject* object, TArray<TSharedPtr<MValue>> parameters, TSharedPtr<MValue>& outRetVal) { \
			/* Using pure black magic, invoke the function with unknown return type, parameter types or count. */ \
			TMethodInvoker<TThisClass, FMethod, &TThisClass::name, TReturn, TMethodParamTypes>::Invoke(static_cast<TThisClass*>(object), parameters, outRetVal); \
		}, \
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
