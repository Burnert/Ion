#include "IonPCH.h"

#include "Reflection.h"
#include "Object.h"

namespace Ion
{
	MType::MType(const MTypeInitializer& initializer) :
		m_Name(initializer.Name),
		m_HashCode(initializer.HashCode),
		m_Size(initializer.Size),
		m_Flags(initializer.Flags)
	{
	}

	MField::MField(const MFieldInitializer& initializer) :
		m_Class(initializer.Class),
		m_FieldType(initializer.FieldType),
		m_FieldOffset(initializer.FieldOffset),
		m_Flags(initializer.Flags),
		m_Name(initializer.Name),
		m_FSetterGetter(initializer.FSetterGetter)
	{
	}

	void MField::SetValue(MObjectPtr object, const MValuePtr& value)
	{
		ionassert(object);
		ionassert(value);

		MReflectionLogger.Trace("Indirectly setting field value {}::{} of object \"{}\".", m_Class->GetName(), m_Name, object->GetName());

		m_FSetterGetter(object, const_cast<MValuePtr&>(value));
	}

	MValuePtr MField::GetValue(MObjectPtr object)
	{
		ionassert(object);

		MReflectionLogger.Trace("Indirectly getting field value {}::{} of object \"{}\".", m_Class->GetName(), m_Name, object->GetName());

		MValuePtr value;
		m_FSetterGetter(object, value);
		return value;
	}

	MMethod::MMethod(const MMethodInitializer& initializer) :
		m_Class(initializer.Class),
		m_ReturnType(initializer.ReturnType),
		m_ParameterTypes(initializer.ParameterTypes),
		m_Flags(initializer.Flags),
		m_Name(initializer.Name),
		m_FInvoke(initializer.FInvoke)
	{
	}

	MValuePtr MMethod::InvokeEx(MObjectPtr object)
	{
		return InvokeEx(object, TArray<MValuePtr>());
	}

	MValuePtr MMethod::InvokeEx(MObjectPtr object, const TArray<MValuePtr>& params)
	{
		MReflectionLogger.Trace("Invoking method {} {}::{}({}) of object \"{}\".", m_ReturnType->GetName(), m_Class->GetName(), m_Name, [this] {
			TArray<String> parameterNames;
			parameterNames.reserve(m_ParameterTypes.size());
			std::transform(m_ParameterTypes.begin(), m_ParameterTypes.end(), std::back_inserter(parameterNames), [](MType* type)
			{
				return type->GetName();
			});
			return JoinString(parameterNames, ", "s);
		}(), object->GetName());

		MValuePtr retValue = nullptr;
		m_FInvoke(object, params, retValue);
		return retValue;
	}

	MClass::MClass(const MClassInitializer& initializer) :
		MType(initializer.TypeInitializer),
		m_SuperClass(initializer.SuperClass),
		m_CDO(initializer.CDO),
		m_FInstantiate(initializer.InstantiateFunc)
	{
	}

	void MClass::SetupClassDefaultObject(const String& name)
	{
		ionassert(m_CDO);
		ionassert(!name.empty());

		// Set the CDO instance MObject fields
		m_CDO->m_Class = this;
		m_CDO->m_Name = name;
		// @TODO: The GUID of a CDO should probably always be the same.
		// Use the version-5 Guid with the class name string as a name.
		m_CDO->m_Guid = GUID();
	}

	MObjectPtr MClass::Instantiate() const
	{
		// @TODO: new is used here anyway (needs to be destroyed at some point)
		// Make this thing use shared ptrs or something
		MObjectPtr object = m_FInstantiate(GetClassDefaultObject());

		// New GUID is needed because it was just copied from the CDO.
		object->m_Guid = GUID();

		return object;
	}

	TArray<MField*> MClass::GetFields() const
	{
		TArray<MField*> fields = m_Fields;
		if (m_SuperClass)
		{
			TArray<MField*> superFields = m_SuperClass->GetFields();
			fields.reserve(fields.size() + superFields.size());
			std::move(superFields.begin(), superFields.end(), fields.end());
		}
		return fields;
	}

	TArray<MMethod*> MClass::GetMethods() const
	{
		// @TODO: Handle overriden method duplication
		TArray<MMethod*> methods = m_Methods;
		if (m_SuperClass)
		{
			TArray<MMethod*> superMethods = m_SuperClass->GetMethods();
			methods.reserve(methods.size() + superMethods.size());
			std::move(superMethods.begin(), superMethods.end(), std::back_inserter(methods));
		}
		return methods;
	}

	Archive& operator<<(Archive& ar, MClass*& mClass)
	{
		XMLArchiveAdapter xmlAr = ar;
		xmlAr.EnterNode("Class");

		String className = ar.IsSaving() ? mClass->GetName() : EmptyString;
		xmlAr << className;

		if (ar.IsLoading())
		{
			mClass = MReflection::FindClassByName(className);
			if (!mClass)
				MReflectionLogger.Error("Class \"{}\" not found.", className);
		}
		xmlAr.ExitNode();

		return ar;
	}

	MEnum::MEnum(const MEnumInitializer& initializer) :
		MType(initializer.TypeInitializer),
		m_UnderlyingType(initializer.UnderlyingType),
		m_FConverter(initializer.FConverter)
	{
	}

	MType* MReflection::RegisterType(const MTypeInitializer& initializer)
	{
		ionassert(!initializer.Name.empty());
		ionassert(initializer.HashCode != 0);

		MType* type = s_ReflectableTypeRegistry.emplace_back(new MType(initializer));

		return type;
	}

	MEnum* MReflection::RegisterEnum(const MEnumInitializer& initializer)
	{
		ionassert(initializer.UnderlyingType);
		ionassert(!initializer.TypeInitializer.Name.empty());
		ionassert(initializer.TypeInitializer.HashCode != 0);

		MEnum* mEnum = s_ReflectableEnumRegistry.emplace_back(new MEnum(initializer));

		return mEnum;
	}

	MClass* MReflection::RegisterClass(const MClassInitializer& initializer)
	{
		// MObject cannot inherit from itself
		ionassert(initializer.SuperClass || initializer.TypeInitializer.Name == "C_MObject");

		ionassert(initializer.CDO);
		ionassert(!initializer.TypeInitializer.Name.empty());
		ionverify(std::find_if(s_MClassRegistry.begin(), s_MClassRegistry.end(), [&](MClass* mc) { return mc->GetName() == initializer.TypeInitializer.Name; }) == s_MClassRegistry.end());

		// Setup the reflectable class data
		MClass* mClass = s_MClassRegistry.emplace_back(new MClass(initializer));
		mClass->SetupClassDefaultObject(initializer.CDOName);
		return mClass;
	}

	MField* MReflection::RegisterField(const MFieldInitializer& initializer)
	{
		ionassert(initializer.Class);
		ionverify(std::find_if(initializer.Class->m_Fields.begin(), initializer.Class->m_Fields.end(), [&](MField* field) { return field->GetName() == initializer.Name; }) == initializer.Class->m_Fields.end());

		TArray<MField*>& fields = initializer.Class->m_Fields;
		MField* field = fields.emplace_back(new MField(initializer));

		//field->InitOffset();

		return field;
	}

	MMethod* MReflection::RegisterMethod(const MMethodInitializer& initializer)
	{
		ionassert(initializer.Class);
		ionverify(std::find_if(initializer.Class->m_Methods.begin(), initializer.Class->m_Methods.end(), [&](MMethod* method) { return method->GetName() == initializer.Name; }) == initializer.Class->m_Methods.end());

		TArray<MMethod*>& methods = initializer.Class->m_Methods;
		MMethod* method = methods.emplace_back(new MMethod(initializer));

		return method;
	}

	MClass* MReflection::FindClassByName(const String& name)
	{
		auto it = std::find_if(s_MClassRegistry.begin(), s_MClassRegistry.end(), [&name](MClass* mClass)
		{
			return mClass->GetName() == name;
		});
		if (it != s_MClassRegistry.end())
			return *it;
		return nullptr;
	}

	MType* MReflection::FindTypeByName(const String& name)
	{
		auto it = std::find_if(s_ReflectableTypeRegistry.begin(), s_ReflectableTypeRegistry.end(), [&name](MType* type)
		{
			return type->GetName() == name;
		});
		if (it != s_ReflectableTypeRegistry.end())
			return *it;
		return nullptr;
	}

	MEnum* MReflection::FindEnumByName(const String& name)
	{
		auto it = std::find_if(s_ReflectableEnumRegistry.begin(), s_ReflectableEnumRegistry.end(), [&name](MEnum* mEnum)
		{
			return mEnum->GetName() == name;
		});
		if (it != s_ReflectableEnumRegistry.end())
			return *it;
		return nullptr;
	}
}
