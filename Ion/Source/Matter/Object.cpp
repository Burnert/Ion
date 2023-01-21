#include "IonPCH.h"

#include "Object.h"
#include "Engine/Engine.h"

namespace Ion
{
	void MObject::Serialize(Archive& ar)
	{
		XMLArchiveAdapter xmlAr = ar;

		xmlAr.EnterNode("Matter");

		xmlAr &= m_Class;

		xmlAr.EnterNode("Name");
		xmlAr &= m_Name;
		xmlAr.ExitNode(); // "Name"

		xmlAr.EnterNode("Guid");
		xmlAr &= m_Guid;
		xmlAr.ExitNode(); // "Guid"

		xmlAr.ExitNode(); // "Matter"
	}

	MObject::MObject() :
		m_Class(nullptr),
		m_Guid(GUID::Zero),
		m_bTickEnabled(false)
	{
		// NOTE: The MObject fields will get set after the constructor is called if the MObject::New method was used.
	}

	// This is here because Engine.h cannot be included in Object.h
	void MObject::Register(const MObjectPtr& object)
	{
		EngineMObjectInterface::RegisterObject(object);
	}

	MObject::~MObject()
	{
		OnDestroy();
	}

	void MObject::SetTickEnabled(bool bEnabled)
	{
		m_bTickEnabled = bEnabled;

		EngineMObjectInterface::SetObjectTickEnabled(SharedFromThis(), bEnabled);
	}

	Archive& operator&=(Archive& ar, MObjectPtr& object)
	{
		XMLArchiveAdapter xmlAr = ar;

		// Retreive and load the class first
		if (ar.IsLoading())
		{
			xmlAr.EnterNode("Matter");
			MClass* mClass;
			ar &= mClass;
			xmlAr.ExitNode();

			// @TODO: Seek() archive to before this read

			if (!mClass)
			{
				// @TODO: Error here
				return ar;
			}

			object = mClass->Instantiate();
		}

		object->Serialize(ar);

		return ar;
	}
}
