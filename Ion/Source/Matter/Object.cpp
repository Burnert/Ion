#include "IonPCH.h"

#include "Object.h"

namespace Ion
{
	void MObject::Serialize(Archive& ar)
	{
		XMLArchiveAdapter xmlAr = ar;

		xmlAr.EnterNode("Matter");

		xmlAr << m_Class;

		xmlAr.EnterNode("Name");
		xmlAr << m_Name;
		xmlAr.ExitNode(); // "Name"

		xmlAr.EnterNode("Guid");
		xmlAr << m_Guid;
		xmlAr.ExitNode(); // "Guid"

		xmlAr.ExitNode(); // "Matter"
	}

	MObject::MObject() :
		m_Class(nullptr),
		m_Guid(GUID::Zero)
	{
		// NOTE: The MObject fields will get set after the constructor is called if the MObject::New method was used.
	}

	MObject::~MObject()
	{
	}

	Archive& operator<<(Archive& ar, MObject*& object)
	{
		XMLArchiveAdapter xmlAr = ar;

		// Retreive and load the class first
		if (ar.IsLoading())
		{
			xmlAr.EnterNode("Matter");
			MClass* mClass;
			ar << mClass;
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
