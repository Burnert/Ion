#include "IonPCH.h"

#include "Reflection.h"
#include "MatterCore.h"

namespace Ion
{
	MType::MType(const type_info& ti) :
		m_HashCode(ti.hash_code())
	{
	}

	MField::MField(MClass* mClass, MType* type) :
		M_Class(mClass),
		m_FieldType(type),
		m_Flags(0)
	{
	}

	MMethod::MMethod(MClass* mClass) :
		M_Class(mClass),
		m_Flags(0)
	{
	}

	MObject* MClass::Instantiate() const
	{
		// @TODO: new is used here anyway
		MObject* object = m_FInstantiate(m_CDO);

		// New GUID is needed because it was just copied from the CDO.
		object->m_Guid = GUID();

		return object;
	}

	MType* MReflection::RegisterType(const type_info& ti, const String& name)
	{
		ionassert(!name.empty());

		MType* type = m_ReflectableTypeRegistry.emplace_back(new MType(ti));
		type->m_Name = name;

		return type;
	}

	MClass* MReflection::RegisterClass(const type_info& ti, const String& name, MObject* cdo, const FMClassInstantiateDefault& instantiate)
	{
		ionassert(cdo);
		ionassert(!name.empty());
		ionverify(std::find_if(m_MClassRegistry.begin(), m_MClassRegistry.end(), [&](MClass* mc) { return mc->GetName() == name; }) == m_MClassRegistry.end());

		// Setup the reflectable class data
		MClass* mClass = m_MClassRegistry.emplace_back(new MClass(ti));
		// C_ prefix to differentiate a class name from an object name.
		mClass->m_Name = "C_" + name;
		mClass->m_CDO = cdo;
		mClass->m_FInstantiate = instantiate;

		// Set the default MObject fields
		mClass->m_CDO->m_Class = mClass;
		mClass->m_CDO->m_Name = name;
		// @TODO: The GUID of a CDO should probably always be the same.
		mClass->m_CDO->m_Guid = GUID();

		return mClass;
	}

	MField* MReflection::RegisterField(MClass* mClass, MType* type, const String& name)
	{
		ionassert(mClass);
		ionverify(std::find_if(mClass->m_Fields.begin(), mClass->m_Fields.end(), [&](MField* field) { return field->GetName() == name; }) == mClass->m_Fields.end());

		MField* field = mClass->m_Fields.emplace_back(new MField(mClass, type));
		field->m_Name = name;

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
