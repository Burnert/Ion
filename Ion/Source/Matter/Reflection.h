#pragma once

#include "Core.h"

#include "ObjectPtr.h"

namespace Ion
{
	REGISTER_LOGGER(MReflectionLogger, "Matter::Reflection");

#pragma region Forward Decl

	class MType;
	class MValue;
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
			Enum        = 1 << 2,
			Void        = 1 << 3,
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
				ETypeFlags::UType bEnum : 1;
				ETypeFlags::UType bVoid : 1;
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

		template<typename T>
		bool IsConvertibleTo() const;
		bool IsConvertibleTo(MType* mType) const;

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
				ETypeFlags::UType m_bEnum : 1;
				ETypeFlags::UType m_bVoid : 1;
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

	using MValuePtr = TSharedPtr<class MValue>;

	class MValue
	{
	public:
		template<typename T>
		static MValuePtr Create(const T& value);

		virtual MType* GetType() const = 0;

		template<typename T>
		const T& As() const
		{
			static_assert(TIsReflectableTypeV<T>);
			ionassert(GetType()->IsConvertibleTo(TGetReflectableType<T>::Type()));
			return *reinterpret_cast<const T*>(GetValuePointer());
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
			static_assert(TIsReflectableTypeV<T>, "Type is not reflectable.");
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
	FORCEINLINE static MValuePtr MValue::Create(const T& value)
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

	using FMFieldSetterGetter = TFunction<void(MObjectPtr, MValuePtr&)>;

	struct MFieldInitializer
	{
		MClass* Class;
		MType* FieldType;
		size_t FieldOffset;
		String Name;
		FMFieldSetterGetter FSetterGetter;
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

		// Indirect setter / getter

		template<typename T>
		void SetValue(MObjectPtr object, const T& value);
		void SetValueEx(MObjectPtr object, const MValuePtr& value);

		template<typename T>
		T GetValue(MObjectPtr object);
		MValuePtr GetValueEx(MObjectPtr object);

		// Direct setter / getter

		template<typename T>
		void SetValueDirect(MObjectPtr object, const T& value);

		template<typename T>
		T GetValueDirect(MObjectPtr object);

	private:
		MField(const MFieldInitializer& initializer);

	private:
		MClass* m_Class;

		MType* m_FieldType;
		size_t m_FieldOffset;

		String m_Name;

		FMFieldSetterGetter m_FSetterGetter;

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
	FORCEINLINE void MField::SetValue(MObjectPtr object, const T& value)
	{
		ionassert(!m_FieldType->IsClass() || TIsObjectPtrV<T>);
		ionassert(m_FieldType->IsClass() || !TIsObjectPtrV<T>);

		SetValueEx(object, MValue::Create(value));
	}

	template<typename T>
	FORCEINLINE T MField::GetValue(MObjectPtr object)
	{
		ionassert(!m_FieldType->IsClass() || TIsObjectPtrV<T>);
		ionassert(m_FieldType->IsClass() || !TIsObjectPtrV<T>);

		return GetValueEx(object)->As<T>();
	}

	template<typename T>
	FORCEINLINE void MField::SetValueDirect(MObjectPtr object, const T& value)
	{
		static_assert(TIsReflectableTypeV<T>, "Type is not reflectable.");
		ionassert(object);
		ionassert(TGetReflectableType<T>::Type()->IsConvertibleTo(m_FieldType), "Type {} is not convertible to {}.",
			TGetReflectableType<T>::Type()->GetName(), m_FieldType->GetName());
		ionassert(!m_FieldType->IsClass() || TIsObjectPtrV<T>);
		ionassert(m_FieldType->IsClass() || !TIsObjectPtrV<T>);
		ionassert(m_FieldType->IsClass() || m_FieldType->GetSize() == sizeof(typename TRemoveObjectPtr<T>::Type));
		ionassert(m_FieldType->IsClass() || m_FieldType->GetHashCode() == typeid(typename TRemoveObjectPtr<T>::Type).hash_code());

		MReflectionLogger.Trace("Setting field value {}::{} of object \"{}\".", m_Class->GetName(), m_Name, object->GetName());

		T* pValue = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(object.Raw()) + m_FieldOffset);
		*pValue = value;
	}

	template<typename T>
	FORCEINLINE T MField::GetValueDirect(MObjectPtr object)
	{
		static_assert(TIsReflectableTypeV<T>, "Type is not reflectable.");
		ionassert(object);
		ionassert(TGetReflectableType<T>::Type()->IsConvertibleTo(m_FieldType), "Type {} is not convertible to {}.",
			TGetReflectableType<T>::Type()->GetName(), m_FieldType->GetName());
		ionassert(!m_FieldType->IsClass() || TIsObjectPtrV<T>);
		ionassert(m_FieldType->IsClass() || !TIsObjectPtrV<T>);
		ionassert(m_FieldType->IsClass() || m_FieldType->GetSize() == sizeof(typename TRemoveObjectPtr<T>::Type));
		ionassert(m_FieldType->IsClass() || m_FieldType->GetHashCode() == typeid(typename TRemoveObjectPtr<T>::Type).hash_code());

		MReflectionLogger.Trace("Getting field value {}::{} of object \"{}\".", m_Class->GetName(), m_Name, object->GetName());

		T* pValue = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(object.Raw()) + m_FieldOffset);
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

	/*                               void(object,     parameters,                 out return value) */
	using FMMethodInvoke = TFunction<void(MObjectPtr, TArray<MValuePtr>, MValuePtr&)>;

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
		TRet Invoke(MObjectPtr object = nullptr, Args&&... args);

		MValuePtr InvokeEx(MObjectPtr object = nullptr);
		MValuePtr InvokeEx(MObjectPtr object, const TArray<MValuePtr>& params);

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
	FORCEINLINE TRet MMethod::Invoke(MObjectPtr object, Args&&... args)
	{
		static_assert(TIsReflectableTypeV<TRet>);
		static_assert(!TIsReferenceV<TRet>, "Invoke function cannot return a reference.");

		MValuePtr retValue = InvokeEx(object, { MValue::Create(args)... });
		if constexpr (!std::is_void_v<TRet>)
		{
			MType* retType = TGetReflectableType<TRet>::Type();
			ionassert(retType);
			ionassert(m_ReturnType->IsConvertibleTo(retType), "Wrong return type has been passed to the Invoke function. {} is not convertible to {}.",
				m_ReturnType->GetName(), retType->GetName());
			return retValue->As<TRet>();
		}
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

	using FMClassInstantiateDefault = TFunction<MObjectPtr(const MObject*)>;

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
		MObjectPtr Instantiate() const;

		const MObject* GetClassDefaultObject() const;

		template<typename T>
		const T& GetClassDefaultObjectTyped() const;

		MClass* GetSuperClass() const;

		TArray<MField*> GetFields() const;
		TArray<MMethod*> GetMethods() const;

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

#pragma endregion

#pragma region Matter Reflection Enum class

	/*                                      err  enumval  asstring */
	using FMEnumStringConverter = TFunction<bool(uint64&, String&)>;

	struct MEnumInitializer
	{
		MTypeInitializer TypeInitializer;
		MType* UnderlyingType;
		FMEnumStringConverter FConverter;
	};

	class MEnum : public MType
	{
	public:
		MType* GetUnderlyingType() const;

	private:
		MEnum(const MEnumInitializer& initializer);

		// @TODO: Get all the possible values and store them here

	private:
		MType* m_UnderlyingType;
		FMEnumStringConverter m_FConverter;

		friend class MReflection;
		friend class MObject;
	};

	FORCEINLINE MType* MEnum::GetUnderlyingType() const
	{
		return m_UnderlyingType;
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
	struct TIsReflectableType<T, TEnableIfT<TIsReflectableClassV<typename TRemoveObjectPtr<T>::Type>>> { static constexpr bool Value = true; };

	template<typename T>
	static inline constexpr bool TIsReflectableTypeV = TIsReflectableType<T>::Value;

	// TGetReflectableType -------------------------------------------------------------------

	template<typename T, typename = void>
	struct TGetReflectableType { static MType* Type() { return nullptr; } };

	template<typename T>
	struct TGetReflectableType<T, TEnableIfT<TIsReflectableClassV<typename TRemoveObjectPtr<T>::Type>>>
	{
		static MType* Type() { return TRemoveObjectPtr<T>::Type::StaticClass(); }
	};

	// TGetReflectableClass -------------------------------------------------------------------

	template<typename T, typename = void>
	struct TGetReflectableClass { static MClass* Class() { return nullptr; } };

	template<typename T>
	struct TGetReflectableClass<T, TEnableIfT<TIsReflectableClassV<typename TRemoveObjectPtr<T>::Type>>>
	{
		static MClass* Class() { return static_cast<MClass*>(TGetReflectableType<T>::Type()); }
	};

	// TGetReflectableSuperClass ------------------------------------------------------------------------

	template<typename T, typename = void>
	struct TGetReflectableSuperClass { static MClass* Class() { return nullptr; } };

	template<typename T>
	struct TGetReflectableSuperClass<T, std::void_t<typename T::Super>> { static MClass* Class() { return T::Super::StaticClass(); } };

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
		static constexpr bool bConstRef = TIsConstReferenceV<TFirst>;

		using Type         = TFirst;
		using StrippedType = TRemoveConstRef<TFirst>;
		using ConstRefType = TIf<bConstRef, TFirst, std::add_lvalue_reference_t<std::add_const_t<TFirst>>>;
		using Rest         = TMethodParamPack<TRest...>;

		static_assert(!TIsReferenceV<Type> || TIsConstReferenceV<Type>, "Non-const reference parameter types are unsupported right now.");
		static_assert(TIsReflectableTypeV<StrippedType>, "The parameter type is not reflectable.");
	};

	template<typename TLast>
	struct TMethodParamPack<TLast>
	{
		static constexpr size_t Count = 1;
		static constexpr bool bConstRef = TIsConstReferenceV<TLast>;

		using Type         = TLast;
		using StrippedType = TRemoveConstRef<TLast>;
		using ConstRefType = TIf<bConstRef, TLast, std::add_lvalue_reference_t<std::add_const_t<TLast>>>;
		using Rest         = TMethodParamPack<>;

		static_assert(!TIsReferenceV<Type> || TIsConstReferenceV<Type>, "Non-const reference parameter types are unsupported right now.");
		static_assert(TIsReflectableTypeV<StrippedType>, "The parameter type is not reflectable.");
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
		static TArray<MType*> Params() { return TArray<MType*> { TGetReflectableType<TRemoveConstRef<TParams>>::Type()... }; }
	};

	// TGetNthParamPack -----------------------------------------------------------------------------

	template<size_t N, typename TParamPack>
	struct TGetNthParamPack { };

	template<size_t N, typename... TParams>
	struct TGetNthParamPack<N, TMethodParamPack<TParams...>>
	{
		using Type = typename TGetNthParamPack<N - 1, typename TMethodParamPack<TParams...>::Rest>::Type;
	};

	template<typename... TParams>
	struct TGetNthParamPack<0, TMethodParamPack<TParams...>>
	{
		using Type = TMethodParamPack<TParams...>;
	};

	template<size_t N>
	struct TGetNthParamPack<N, TMethodParamPack<>>
	{
		using Type = TMethodParamPack<>;
	};

	// TGetNthParamType -------------------------------------------------------------------------

	template<size_t N, typename TParamPack>
	struct TGetNthParamType
	{
		using Type = typename TGetNthParamPack<N, TParamPack>::Type::Type;
	};

	template<size_t N>
	struct TGetNthParamType<N, TMethodParamPack<>>
	{
		using Type = void;
	};

	// TMethodInvoker ----------------------------------------------------------------------------
	
	template<typename TClass, typename TFuncPtr, TFuncPtr Func, typename TReturn, typename TParamPack>
	struct TMethodInvoker
	{
		FORCEINLINE static void Invoke(
			const TObjectPtr<TClass>& object,
			const TArray<MValuePtr>& parameters,
			MValuePtr& outRetVal)
		{
			ionassert(object); // @TODO: Unless static
			ionassert(parameters.size() == TParamPack::Count,
				"A wrong amount of parameters has been passed to the invoke function. {} instead of {}.",
				parameters.size(), TParamPack::Count);

			Invoke_Impl(object.Raw(), parameters, outRetVal, std::make_index_sequence<TParamPack::Count> {});
		}

	private:
		template<size_t... I>
		FORCEINLINE static void Invoke_Impl(
			TClass* object,
			const TArray<MValuePtr>& parameters,
			MValuePtr& outRetVal,
			std::index_sequence<I...> seq)
		{
			static_assert(sizeof...(I) == TParamPack::Count);
			ionassert(sizeof...(I) == parameters.size());

			if constexpr (!std::is_void_v<TReturn>)
			{
				outRetVal = MValue::Create<TRemoveConstRef<TReturn>>( (object->*Func)(ExtractParamForInvoke<I>(parameters)...) );
			}
			else
			{
				(object->*Func)(ExtractParamForInvoke<I>(parameters)...);
				outRetVal = nullptr;
			}
		}

		template<size_t Index>
		FORCEINLINE static auto& ExtractParamForInvoke(const TArray<MValuePtr>& parameters)
		{
			using ParamType = typename TGetNthParamPack<Index, TParamPack>::Type::StrippedType;

			const MValuePtr& value = parameters[Index];
			ionassert(Index < parameters.size());
			ionassert(value->GetType()->IsConvertibleTo(TGetReflectableType<ParamType>::Type()),
				"Wrong parameter type has been passed at index {}. {} is not convertible to {}.",
				Index, value->GetType()->GetName(), TGetReflectableType<ParamType>::Type()->GetName());

			return value->As<ParamType>();
		}
	};

#pragma endregion

#pragma region Matter Type IsConvertibleTo implementation

	template<typename T>
	FORCEINLINE bool MType::IsConvertibleTo() const
	{
		static_assert(TIsReflectableTypeV<T>);
		return IsConvertibleTo(TGetReflectableType<T>::Type());
	}

	FORCEINLINE bool MType::IsConvertibleTo(MType* mType) const
	{
		ionassert(mType);

		if (Is(mType))
			return true;

		if (IsClass() && mType->IsClass())
		{
			MClass* fromClass = static_cast<MClass*>(const_cast<MType*>(this));
			for (MClass* superClass = fromClass->GetSuperClass(); superClass; superClass = superClass->GetSuperClass())
			{
				if (superClass->Is(mType))
					return true;
			}
		}

		// @TODO: Implement fundamental types
		return false;
	}

#pragma endregion

#pragma region Matter Reflection Registry

// Damn you Windows
#undef RegisterClass

	class ION_API MReflection
	{
	public:
		static MType* RegisterType(const MTypeInitializer& initializer);
		static MEnum* RegisterEnum(const MEnumInitializer& initializer);
		static MClass* RegisterClass(const MClassInitializer& initializer);
		static MField* RegisterField(const MFieldInitializer& initializer);
		static MMethod* RegisterMethod(const MMethodInitializer& initializer);

		static MClass* FindClassByName(const String& name);
		static MType* FindTypeByName(const String& name);
		static MEnum* FindEnumByName(const String& name);

	private:
		static inline TArray<MType*> s_ReflectableTypeRegistry;
		static inline TArray<MEnum*> s_ReflectableEnumRegistry;
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

#define MATTER_DECLARE_CLASS(T) \
public: \
using TThisClass = T; \
T(const T&) = default; \
T(T&&) = default; \
TObjectPtr<TThisClass> This() { return PtrCast<TThisClass>(SharedFromThis()); } \
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
		TGetReflectableSuperClass<T>::Class(), \
		/* Instantiate function - Copy construct a new object from the CDO instance. */ \
		[](const MObject* cdo) -> MObjectPtr { \
			return MakeShared<T>(*static_cast<const T*>(cdo)); \
		} \
	}; \
	static MClass* c_Class = MReflection::RegisterClass(c_Initializer); \
	return c_Class; \
} \
static inline MClass* const MatterRT = StaticClass();

#define MCLASS(T) \
MATTER_DECLARE_CLASS(T); \
/* Implement archive operator */ \
FORCEINLINE friend Archive& operator<<(Archive& ar, TObjectPtr<T>& value) { \
	ar << SerializeMObject(value); \
	return ar; \
}

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
		/* MField FSetterGetter function */ \
		[](MObjectPtr object, MValuePtr& valueRef) { \
			/* If the pointer is not null, the function should be used as a setter. */ \
			if (valueRef) \
				PtrCast<TThisClass>(object).Raw()->*(&TThisClass::name) = valueRef->As<TField>(); \
			else \
				valueRef = MakeShared<TMValue<TField>>(PtrCast<TThisClass>(object).Raw()->*(&TThisClass::name)); \
		}, \
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
	static_assert(!TIsReferenceV<TReturn> || TIsConstReferenceV<TReturn>, "Non-const reference return types are unsupported right now."); \
	static_assert(TIsReflectableTypeV<TRemoveConstRef<TReturn>>, "The return type is not reflectable."); \
	static TArray<MType*> parameterTypes = TMethodGetReflectableParamTypes<TMethodParamTypes>::Params(); \
	static MType* returnType = TGetReflectableType<TRemoveConstRef<TReturn>>::Type(); \
	ionassert(std::all_of(parameterTypes.begin(), parameterTypes.end(), [](MType* type) { return type; })); \
	static MMethodInitializer c_Initializer { \
		/* The owning class */ \
		StaticClass(), \
		/* Method return type */ \
		returnType, \
		/* Method parameter types */ \
		parameterTypes, \
		/* The name of the method */ \
		#name, \
		/* MMethod FInvoke function */ \
		[](MObjectPtr object, TArray<MValuePtr> parameters, MValuePtr& outRetVal) { \
			/* Using pure black magic, invoke the function with "unknown" return type, parameter types or count. */ \
			TMethodInvoker<TThisClass, FMethod, &TThisClass::name, TReturn, TMethodParamTypes>::Invoke(PtrCast<TThisClass>(object), parameters, outRetVal); \
		}, \
		/* Attributes (accessibility, static, etc.) */ \
		EMethodFlags::None \
	}; \
	return MReflection::RegisterMethod(c_Initializer); \
}();

#define MENUM(T) \
static inline MEnum* MatterRE_##T = [] { \
	using TUnderlying = std::underlying_type_t<T>; \
	using TParser = TEnumParser<T>; \
	static MEnumInitializer c_Initializer { \
		MTypeInitializer { \
			/* The MEnum name (E_ prefix) */ \
			"E_"s + #T, \
			/* The compiler provided type hash code */ \
			typeid(T).hash_code(), \
			/* Type size in bytes */ \
			sizeof(T), \
			/* Type attributes */ \
			ETypeFlags::Enum \
		}, \
		/* Underlying enum type */ \
		TGetReflectableType<TUnderlying>::Type(), \
		/* 2-way conversion function */ \
		[](uint64& value, String& sValue) -> bool { \
			/* If the string is empty, apply ToString conversion */ \
			if (sValue.empty()) { \
				sValue = TParser::ToString((T)value); \
				return !sValue.empty(); \
			} \
			else { \
				TOptional<T> opt = TParser::FromString(sValue); \
				if (opt) value = (uint64)*opt; \
				return (bool)opt; \
			} \
		} \
	}; \
	return MReflection::RegisterEnum(c_Initializer); \
}(); \
template<> struct TGetReflectableType<T> { static MType* Type() { return MatterRE_##T; } }; \
template<> struct TIsReflectableType<T> { static constexpr bool Value = true; };

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

	/* Custom void type (can't be used as a field or parameter, only method return type) */
	inline MType* const MatterRT_void = [] {
		static MTypeInitializer c_Initializer { };
		c_Initializer.Name = "void";
		c_Initializer.HashCode = typeid(void).hash_code();
		c_Initializer.Size = 0;
		c_Initializer.Flags = ETypeFlags::Void;
		return MReflection::RegisterType(c_Initializer);
	}();
	template<> struct TGetReflectableType<void> { static MType* Type() { return MatterRT_void; } };
	template<> struct TIsReflectableType<void> { static constexpr bool Value = true; };

#pragma endregion

#pragma region Special Reflectable Types

	// @TODO: Pointers? Might require wrappers

	// @TODO: Collections:
	// - Dynamic Array
	// - Fixed Array
	// - Hash Set
	// - Hash Map

	MTYPE(GUID)
	MTYPE(String)

#pragma endregion
}
