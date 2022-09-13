#pragma once

#include "Core.h"

namespace Ion
{
	class MType;
	class MField;
	class MMethod;
	class MClass;
	class MReflection;
	class MObject;

#pragma region Matter Reflection Type class

	class MType
	{
	public:
		const String& GetName() const;

		template<typename T>
		bool Is() const;

		bool Is(MType* mType) const;

	protected:
		MType(const type_info& ti);

	private:
		String m_Name;
		size_t m_HashCode;

		friend class MReflection;
	};

	FORCEINLINE const String& MType::GetName() const
	{
		return m_Name;
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
			Public,
			Protected,
			Private,
			Static,
		};
	}

	class MField
	{
	public:
		const String& GetName() const;

	private:
		MField(MClass* mClass, MType* type);

	private:
		MClass* M_Class;
		MType* m_FieldType;

		String m_Name;

#if ION_DEBUG
		union {
#endif
		std::underlying_type_t<EFieldFlags::Type> m_Flags;
#if ION_DEBUG // Flags debug visualization
			struct
			{
				uint32 m_bPublic : 1;
				uint32 m_bProtected : 1;
				uint32 m_bPrivate : 1;
				uint32 m_bStatic : 1;
			};
		};
#endif
		friend class MReflection;
	};

	FORCEINLINE const String& MField::GetName() const
	{
		return m_Name;
	}

#pragma endregion

#pragma region Matter Reflection Method class

	namespace EMethodFlags
	{
		enum Type : uint32
		{
			Public,
			Protected,
			Private,
			Static,
			Virtual,
		};
	}

	class MMethod
	{
	public:
		template<typename... Args>
		void Invoke(Args&&... args);

		const String& GetName() const;

	private:
		MMethod(MClass* mClass);

	private:
		MClass* M_Class;

		String m_Name;

#if ION_DEBUG
		union {
#endif
		std::underlying_type_t<EMethodFlags::Type> m_Flags;
#if ION_DEBUG // Flags debug visualization
			struct
			{
				uint8 m_bPublic : 1;
				uint8 m_bProtected : 1;
				uint8 m_bPrivate : 1;
				uint8 m_bStatic : 1;
				uint8 m_bVirtual : 1;
			};
		};
#endif
		friend class MReflection;
	};

	FORCEINLINE const String& MMethod::GetName() const
	{
		return m_Name;
	}

#pragma endregion

#pragma region Matter Reflection Class class

	using FMClassInstantiateDefault = TFunction<MObject* (MObject*)>;

	class MClass : public MType
	{
	public:
		const String& GetName() const;

		MObject* Instantiate() const;

	private:
		template<typename T>
		FORCEINLINE T& GetClassDefaultObject()
		{
			return *reinterpret_cast<T*>(m_CDO);
		}

		FORCEINLINE MClass(const type_info& ti) :
			MType(ti),
			m_CDO(nullptr)
		{
		}

	private:
		String m_Name;

		TArray<MField*> m_Fields;
		TArray<MMethod*> m_Methods;

		MObject* m_CDO;
		FMClassInstantiateDefault m_FInstantiate;

		friend class MReflection;
		friend class MObject;

	public:
		FORCEINLINE friend Archive& operator<<(Archive& ar, MClass*& mClass)
		{
			XMLArchiveAdapter xmlAr = ar;
			xmlAr.EnterNode("Class");
			if (ar.IsSaving())
			{
				String className = mClass->GetName();
				xmlAr << className;
			}
			else if (ar.IsLoading())
			{
				// @TODO: Query and load a class
			}
			xmlAr.ExitNode();

			return ar;
		}
	};
	
	FORCEINLINE const String& MClass::GetName() const
	{
		return m_Name;
	}

#pragma endregion

#pragma region Matter Reflection Class Registry

#define MTYPE(T) inline MType* const MatterRT_##T = MReflection::RegisterType(typeid(T), #T)

#define MCLASS(T) \
public: \
FORCEINLINE static MClass* StaticClass() { \
	static MClass* c_Class = MReflection::RegisterClass(typeid(T), #T, new T, \
	/* Copy construct a new object from the CDO instance. */ \
	[](MObject* cdo) { \
		return new T(*static_cast<T*>(cdo)); \
	}); \
	return c_Class; \
}

#define MFIELD(T, name)

#define MMETHOD(...)

	class ION_API MReflection
	{
	public:
		static MType* RegisterType(const type_info& ti, const String& name);
		static MClass* RegisterClass(const type_info& ti, const String& name, MObject* cdo, const FMClassInstantiateDefault& instantiate);
		static MField* RegisterField(MClass* mClass, MType* type, const String& name);
		static MMethod* RegisterMethod(MClass* mClass, const String& name);

	private:
		static inline TArray<MType*> m_ReflectableTypeRegistry;
		static inline TArray<MClass*> m_MClassRegistry;
	};

#pragma endregion

#pragma region Fundamental Reflectable Types

	MTYPE(int8);
	MTYPE(int16);
	MTYPE(int32);
	MTYPE(int64);
	MTYPE(uint8);
	MTYPE(uint16);
	MTYPE(uint32);
	MTYPE(uint64);
	MTYPE(bool);
	MTYPE(float);
	MTYPE(double);

	// @TODO: Pointers? Might require wrappers

#pragma endregion
}
