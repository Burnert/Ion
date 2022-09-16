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

		MObject* MObjectField = nullptr;
		MFIELD(MObjectField)

		void Void_VoidMethod() { MReflectionLogger.Debug("Void_VoidMethod called"); }
		MMETHOD(Void_VoidMethod)

		int32 Int_VoidMethod() { MReflectionLogger.Debug("Int_VoidMethod called"); return 420; }
		MMETHOD(Int_VoidMethod)

		int32 Int_IntMethod(int32 param) { MReflectionLogger.Debug("Int_IntMethod called {}", param); return param; }
		MMETHOD(Int_IntMethod, int32)

		MObject* MObject_IntMethod(int32 param) { MReflectionLogger.Debug("MObject_IntMethod called {}", param); return this; }
		MMETHOD(MObject_IntMethod, int32)

		MObject* MObject_MObjectMethod(MObject* param) { MReflectionLogger.Debug("MObject_MObjectMethod called {}", (void*)param); return param; }
		MMETHOD(MObject_MObjectMethod, MObject*)

		MObject* MObject_MObjectIntMethod(MObject* param, int32 param2) { MReflectionLogger.Debug("MObject_MObjectIntMethod called {}, {}", (void*)param, param2); return param; }
		MMETHOD(MObject_MObjectIntMethod, MObject*, int32)
	};

	void MatterTest()
	{
		MMatterTest* object = MObject::New<MMatterTest>();
		ionassert(object);

		MClass* c = object->GetClass();
		ionassert(c);
		ionassert(c->GetName() == "C_MMatterTest");
		ionassert(c->Is<MMatterTest>());
		ionassert(c->Is(MMatterTest::StaticClass()));

		// Fields

		ionassert(MMatterTest::MatterRF_IntField->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRF_IntField->GetType() == MatterRT_int32);
		ionassert(MMatterTest::MatterRF_IntField->GetType()->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRF_IntField->GetType()->GetSize() == sizeof(int32));
		ionassert(!MMatterTest::MatterRF_IntField->GetType()->IsClass());
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
		ionassert(!MMatterTest::MatterRF_IntField2->GetType()->IsClass());
		ionassert(MMatterTest::MatterRF_IntField2->GetName() == "IntField2");
		ionassert(MMatterTest::MatterRF_IntField2->GetOffset() == offsetof(MMatterTest, IntField2));

		ionassert(MMatterTest::MatterRF_MObjectField->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRF_MObjectField->GetType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRF_MObjectField->GetType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRF_MObjectField->GetType()->GetSize() == sizeof(MObject));
		ionassert(MMatterTest::MatterRF_MObjectField->GetType()->IsClass());
		ionassert(MMatterTest::MatterRF_MObjectField->GetName() == "MObjectField");
		ionassert(MMatterTest::MatterRF_MObjectField->GetOffset() == offsetof(MMatterTest, MObjectField));

		MObject* object2 = MObject::New<MMatterTest>();
		MMatterTest::MatterRF_MObjectField->SetValue(object, object2);
		ionassert(object->MObjectField == object2);
		ionassert(object->MObjectField == MMatterTest::MatterRF_MObjectField->GetValue<MObject*>(object));

		// Methods

		ionassert(MMatterTest::MatterRM_Void_VoidMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_Void_VoidMethod->GetReturnType() == MatterRT_void);
		ionassert(MMatterTest::MatterRM_Void_VoidMethod->GetReturnType()->Is(MatterRT_void));
		ionassert(MMatterTest::MatterRM_Void_VoidMethod->GetParameterTypes().empty());
		ionassert(MMatterTest::MatterRM_Void_VoidMethod->GetName() == "Void_VoidMethod");

		MMatterTest::MatterRM_Void_VoidMethod->InvokeEx(object);
		MMatterTest::MatterRM_Void_VoidMethod->Invoke(object);

		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetReturnType() == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetReturnType()->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetParameterTypes().empty());
		ionassert(MMatterTest::MatterRM_Int_VoidMethod->GetName() == "Int_VoidMethod");

		TSharedPtr<MValue> intRet0 = MMatterTest::MatterRM_Int_VoidMethod->InvokeEx(object);
		ionassert(intRet0->As<int32>() == 420);

		int32 intRet0x = MMatterTest::MatterRM_Int_VoidMethod->Invoke<int32>(object);
		ionassert(intRet0x == 420);

		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetReturnType() == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetReturnType()->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetParameterTypes()[0] == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetParameterTypes()[0]->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_Int_IntMethod->GetName() == "Int_IntMethod");

		TSharedPtr<MValue> intRet1 = MMatterTest::MatterRM_Int_IntMethod->InvokeEx(object, { MValue::Create(2137) });
		ionassert(intRet1->As<int32>() == 2137);

		int32 intRet1x = MMatterTest::MatterRM_Int_IntMethod->Invoke<int32>(object, 2137i32);
		ionassert(intRet1x == 2137);

		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetReturnType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetReturnType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetParameterTypes()[0] == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetParameterTypes()[0]->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_MObject_IntMethod->GetName() == "MObject_IntMethod");

		TSharedPtr<MValue> mObjectRet0 = MMatterTest::MatterRM_MObject_IntMethod->InvokeEx(object, { MValue::Create(2137) });
		ionassert(mObjectRet0->As<MObject*>() == object);

		MObject* mObjectRet0x = MMatterTest::MatterRM_MObject_IntMethod->Invoke<MObject*>(object, 2137i32);
		ionassert(mObjectRet0x == object);

		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetReturnType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetReturnType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetParameterTypes()[0] == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetParameterTypes()[0]->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectMethod->GetName() == "MObject_MObjectMethod");

		TSharedPtr<MValue> mObjectRet1 = MMatterTest::MatterRM_MObject_MObjectMethod->InvokeEx(object, { MValue::Create(object2) });
		ionassert(mObjectRet1->As<MObject*>() == object2);

		MObject* mObjectRet1x = MMatterTest::MatterRM_MObject_MObjectMethod->Invoke<MObject*>(object, object2);
		ionassert(mObjectRet1x == object2);

		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetReturnType() == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetReturnType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes().size() == 2);
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[0] == MObject::StaticClass());
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[0]->Is(MObject::StaticClass()));
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[1] == MatterRT_int32);
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetParameterTypes()[1]->Is(MatterRT_int32));
		ionassert(MMatterTest::MatterRM_MObject_MObjectIntMethod->GetName() == "MObject_MObjectIntMethod");

		TSharedPtr<MValue> mObjectRet2 = MMatterTest::MatterRM_MObject_MObjectIntMethod->InvokeEx(object, { MValue::Create(object2), MValue::Create(69) });
		ionassert(mObjectRet2->As<MObject*>() == object2);

		MObject* mObjectRet2x = MMatterTest::MatterRM_MObject_MObjectIntMethod->Invoke<MObject*>(object, object2, 69i32);
		ionassert(mObjectRet2x == object2);

		// Iteration

		TArray<MField*> fields = c->GetFields();
		ionassert(fields.size() == 3);
		for (MField* field : fields)
		{
			ionassert(field->GetClass() == c);
		}

		TArray<MMethod*> methods = c->GetMethods();
		ionassert(methods.size() == 6);
		for (MMethod* method : methods)
		{
			ionassert(method->GetClass() == c);
		}

		ionbreak();
	}
}
