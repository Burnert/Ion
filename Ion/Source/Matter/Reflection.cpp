#include "IonPCH.h"

#include "Reflection.h"
#include "Object.h"

namespace Ion
{
	MType::MType(const MTypeInitializer& initializer) :
		m_Name(initializer.Name),
		m_HashCode(initializer.HashCode)
	{
	}

	MField::MField(const MFieldInitializer& initializer) :
		m_Class(initializer.Class),
		m_FieldType(initializer.FieldType),
		m_Flags(initializer.Flags),
		m_Name(initializer.Name)
	{
	}

	MMethod::MMethod(const MMethodInitializer& initializer) :
		m_Class(initializer.Class),
		m_ReturnType(initializer.ReturnType),
		m_ParameterTypes(initializer.ParameterTypes),
		m_Flags(initializer.Flags),
		m_Name(initializer.Name)
	{
	}

	MClass::MClass(const MClassInitializer& initializer) :
		// C_ prefix to differentiate a class name from an object name.
		MType(MTypeInitializer { "C_" + initializer.Name, initializer.TypeHashCode }),
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

	MObject* MClass::Instantiate() const
	{
		// @TODO: new is used here anyway (needs to be destroyed at some point)
		// Make this thing use shared ptrs or something
		MObject* object = m_FInstantiate(GetClassDefaultObject());

		// New GUID is needed because it was just copied from the CDO.
		object->m_Guid = GUID();

		return object;
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

	MType* MReflection::RegisterType(const MTypeInitializer& initializer)
	{
		ionassert(!initializer.Name.empty());
		ionassert(initializer.HashCode != 0);

		MType* type = s_ReflectableTypeRegistry.emplace_back(new MType(initializer));

		return type;
	}

	MClass* MReflection::RegisterClass(const MClassInitializer& initializer)
	{
		// MObject cannot inherit from itself
		ionassert(initializer.SuperClass || initializer.Name == "MObject");

		ionassert(initializer.CDO);
		ionassert(!initializer.Name.empty());
		ionverify(std::find_if(s_MClassRegistry.begin(), s_MClassRegistry.end(), [&](MClass* mc) { return mc->GetName() == initializer.Name; }) == s_MClassRegistry.end());

		// Setup the reflectable class data
		MClass* mClass = s_MClassRegistry.emplace_back(new MClass(initializer));
		mClass->SetupClassDefaultObject(initializer.Name);
		return mClass;
	}

	MField* MReflection::RegisterField(const MFieldInitializer& initializer)
	{
		ionassert(initializer.Class);
		ionverify(std::find_if(initializer.Class->m_Fields.begin(), initializer.Class->m_Fields.end(), [&](MField* field) { return field->GetName() == initializer.Name; }) == initializer.Class->m_Fields.end());

		TArray<MField*>& fields = initializer.Class->m_Fields;
		MField* field = fields.emplace_back(new MField(initializer));

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
}
