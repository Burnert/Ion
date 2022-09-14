#include "IonPCH.h"

#include "Reflection.h"
#include "MObject.h"

namespace Ion
{
	MType::MType(const MTypeInitializer& initializer) :
		m_Name(initializer.Name),
		m_HashCode(initializer.HashCode)
	{
	}

	MField::MField(const MFieldInitializer& initializer) :
		M_Class(initializer.Class),
		m_FieldType(initializer.FieldType),
		m_Flags(initializer.Flags),
		m_Name(initializer.Name)
	{
	}

	MMethod::MMethod(MClass* mClass) :
		m_Class(mClass),
		m_ReturnType(nullptr),
		m_Flags(0)
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
		MObject* object = m_FInstantiate(m_CDO);

		// New GUID is needed because it was just copied from the CDO.
		object->m_Guid = GUID();

		return object;
	}

	MType* MReflection::RegisterType(const MTypeInitializer& initializer)
	{
		ionassert(!initializer.Name.empty());
		ionassert(initializer.HashCode != 0);

		MType* type = m_ReflectableTypeRegistry.emplace_back(new MType(initializer));

		return type;
	}

	MClass* MReflection::RegisterClass(const MClassInitializer& initializer)
	{
		// MObject cannot inherit from itself
		ionassert(initializer.SuperClass || initializer.Name == "MObject");

		ionassert(initializer.CDO);
		ionassert(!initializer.Name.empty());
		ionverify(std::find_if(m_MClassRegistry.begin(), m_MClassRegistry.end(), [&](MClass* mc) { return mc->GetName() == initializer.Name; }) == m_MClassRegistry.end());

		// Setup the reflectable class data
		MClass* mClass = m_MClassRegistry.emplace_back(new MClass(initializer));
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

	MMethod* MReflection::RegisterMethod(MClass* mClass, const String& name)
	{
		ionassert(mClass);
		ionverify(std::find_if(mClass->m_Methods.begin(), mClass->m_Methods.end(), [&](MMethod* method) { return method->GetName() == name; }) == mClass->m_Methods.end());

		MMethod* method = mClass->m_Methods.emplace_back(new MMethod(mClass));
		method->m_Name = name;

		return method;
	}
}
