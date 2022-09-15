#include "IonPCH.h"

#include "Object.h"

namespace Ion::Test
{
	class MMatterTest : public MObject
	{
		MCLASS(MMatterTest)
		using Super = MObject;

		int32 IntField = 0;
		MFIELD(IntField)

		int64 IntField2 = 1;
		MFIELD(IntField2)

		int32 Int_VoidMethod() { return 0; }
		MMETHOD(Int_VoidMethod)

		int32 Int_IntMethod(int32 param) { return 0; }
		MMETHOD(Int_IntMethod, int32)

		MObject* MObject_IntMethod(int32 param) { return nullptr; }
		MMETHOD(MObject_IntMethod, int32)

		MObject* MObject_MObjectMethod(MObject* param) { return nullptr; }
		MMETHOD(MObject_MObjectMethod, MObject*)

		MObject* MObject_MObjectIntMethod(MObject* param, int32 param2) { return nullptr; }
		MMETHOD(MObject_MObjectIntMethod, MObject*, int32)
	};

	void MatterTest()
	{
		MMatterTest* object = MObject::New<MMatterTest>();
		ionassert(object);

		sizeof(MObject);

		MClass* c = object->GetClass();
		ionassert(c);
		ionassert(c->GetName() == "C_MMatterTest");
		ionassert(c->Is<MMatterTest>());
		ionassert(c->Is(MMatterTest::StaticClass()));

		ionassert(MMatterTest::MatterRF_IntField->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRF_IntField->GetType() == MatterRT_int32);
		ionassert(MMatterTest::MatterRF_IntField->GetType()->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRF_IntField->GetType()->GetSize() == sizeof(int32));
		ionassert(MMatterTest::MatterRF_IntField->GetName() == "IntField");
		ionassert(MMatterTest::MatterRF_IntField->GetOffset() == offsetof(MMatterTest, IntField));

		int32 value0 = MMatterTest::MatterRF_IntField->GetValue<int32>(object);
		ionassert(value0 == object->IntField);
		MMatterTest::MatterRF_IntField->SetValue(object, (int32)69);
		ionassert(object->IntField == 69);

		ionassert(MMatterTest::MatterRF_IntField2->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRF_IntField2->GetType() == MatterRT_int64);
		ionassert(MMatterTest::MatterRF_IntField2->GetType()->Is(MatterRT_int64));
		ionassert(MMatterTest::MatterRF_IntField2->GetType()->GetSize() == sizeof(int64));
		ionassert(MMatterTest::MatterRF_IntField2->GetName() == "IntField2");

		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetReturnType() == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetReturnType()->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetParameterTypes().empty());
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetName() == "Int_VoidMethod");

		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetReturnType() == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetReturnType()->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetParameterTypes()[0] == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetParameterTypes()[0]->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetName() == "Int_IntMethod");

		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetReturnType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetReturnType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetParameterTypes()[0] == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetParameterTypes()[0]->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetName() == "MObject_IntMethod");

		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetReturnType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetReturnType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetParameterTypes()[0] == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetParameterTypes()[0]->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetName() == "MObject_MObjectMethod");

		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetReturnType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetReturnType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes().size() == 2);
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[0] == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[0]->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[1] == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[1]->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetName() == "MObject_MObjectIntMethod");

		TArray<MField*> fields = c->GetFields();
		ionassert(fields.size() == 2);
		for (MField* field : fields)
		{
			ionassert(field->GetClass() == c);
		}

		TArray<MMethod*> methods = c->GetMethods();
		ionassert(methods.size() == 5);
		for (MMethod* method : methods)
		{
			ionassert(method->GetClass() == c);
		}

		ionbreak();
	}
}
