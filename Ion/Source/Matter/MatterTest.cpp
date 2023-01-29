#include "IonPCH.h"

#include "Object.h"

namespace Ion
{
	// @TODO: FIX Enums only work in Ion namespace

	enum class EMatterEnum
	{
		Null = 0,
		Value1,
		Value2,
		Value3,
		Value4,
	};

	template<>
	struct TEnumParser<EMatterEnum>
	{
		ENUM_PARSER_TO_STRING_BEGIN(EMatterEnum)
		ENUM_PARSER_TO_STRING_HELPER(Null)
		ENUM_PARSER_TO_STRING_HELPER(Value1)
		ENUM_PARSER_TO_STRING_HELPER(Value2)
		ENUM_PARSER_TO_STRING_HELPER(Value3)
		ENUM_PARSER_TO_STRING_HELPER(Value4)
		ENUM_PARSER_TO_STRING_END()

		ENUM_PARSER_FROM_STRING_BEGIN(EMatterEnum)
		ENUM_PARSER_FROM_STRING_HELPER(Null)
		ENUM_PARSER_FROM_STRING_HELPER(Value1)
		ENUM_PARSER_FROM_STRING_HELPER(Value2)
		ENUM_PARSER_FROM_STRING_HELPER(Value3)
		ENUM_PARSER_FROM_STRING_HELPER(Value4)
		ENUM_PARSER_FROM_STRING_END()
	};

	MENUM(EMatterEnum)
}
namespace Ion::Test
{
#pragma region Matter generic test

	class MMatterTest : public MObject
	{
		MCLASS(MMatterTest)
		using Super = MObject;

		MMatterTest()
		{
		}

		int32 IntField = 0;
		MFIELD(IntField)

		int64 IntField2 = 1;
		MFIELD(IntField2)

		MObjectPtr MObjectField = nullptr;
		MFIELD(MObjectField)

		EMatterEnum EnumField = EMatterEnum::Value2;
		MFIELD(EnumField)

		TArray<int32> ArrayField;
		MFIELD(ArrayField)

		THashMap<int32, String> HashMapField;
		MFIELD(HashMapField)

		void Void_VoidMethod() { MReflectionLogger.Debug("Void_VoidMethod called"); }
		MMETHOD(Void_VoidMethod)

		void Void_ConstRefIntMethod(const int32& param) { MReflectionLogger.Debug("Void_ConstRefIntMethod called {}", param); }
		MMETHOD(Void_ConstRefIntMethod, const int32&)

		int32 Int420 = 420;
		const int32& Int_VoidMethod() { MReflectionLogger.Debug("Int_VoidMethod called"); return Int420; }
		MMETHOD(Int_VoidMethod)

		int32 Int_IntMethod(int32 param) { MReflectionLogger.Debug("Int_IntMethod called {}", param); return param; }
		MMETHOD(Int_IntMethod, int32)

		TObjectPtr<MMatterTest> MMatterTest_IntMethod(int32 param) { MReflectionLogger.Debug("MMatterTest_IntMethod called {}", param); return This(); }
		MMETHOD(MMatterTest_IntMethod, int32)

		MObjectPtr MObject_MObjectMethod(MObjectPtr const& param) { MReflectionLogger.Debug("MObject_MObjectMethod called {}", (void*)param.Raw()); return param; }
		MMETHOD(MObject_MObjectMethod, MObjectPtr const&)

		MObjectPtr MObject_MObjectIntMethod(MObjectPtr param, int32 param2) { MReflectionLogger.Debug("MObject_MObjectIntMethod called {}, {}", (void*)param.Raw(), param2); return param; }
		MMETHOD(MObject_MObjectIntMethod, MObjectPtr, int32)

		void Void_StringMethod(const String& str) { MReflectionLogger.Debug("Void_StringMethod called {}", str); }
		MMETHOD(Void_StringMethod, const String&)

		String Str420 = "-__420__-";
		const String& String_VoidMethod() { MReflectionLogger.Debug("String_VoidMethod called"); return Str420; }
		MMETHOD(String_VoidMethod)

		const GUID& GUID_GUIDMethod(const GUID& guid) { MReflectionLogger.Debug("GUID_GUIDMethod called {}", guid.ToString()); return guid; }
		MMETHOD(GUID_GUIDMethod, const GUID&)

		const TArray<int32>& Array_ArrayMethod(const TArray<int32>& array) { MReflectionLogger.Debug("Array_ArrayMethod called {}", [&] { std::stringstream ss; for (int32 i : array) ss << i << ","; return ss.str(); }()); return array; };
		MMETHOD(Array_ArrayMethod, const TArray<int32>&)

		const THashMap<int32, String>& HashMap_HashMapMethod(const THashMap<int32, String>& map) { MReflectionLogger.Debug("HashMap_HashMapMethod called {}", [&] { std::stringstream ss; for (const auto& [k, v] : map) ss << k << ":" << v << ","; return ss.str(); }()); return map; }
		MMETHOD(HashMap_HashMapMethod, const THashMap<int32, String>&)

		void Void_IntRefMethod(int32& intRef) { MReflectionLogger.Debug("Void_IntRefMethod called"); intRef = Int420; }
		MMETHOD(Void_IntRefMethod, int32&)
	};

	void MatterClassTest()
	{
		TObjectPtr<MMatterTest> object = MObject::New<MMatterTest>();
		ionassert(object);

		MClass* c = object->GetClass();
		ionassert(c);
		ionassert(c->GetName() == "C_MMatterTest");
		ionassert(c->Is<MMatterTest>());
		ionassert(c->Is(MMatterTest::StaticClass()));

		// Fields

		ionassert(MMatterTest::__RF_IntField->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RF_IntField->GetType() == __RT_int32);
		ionassert(MMatterTest::__RF_IntField->GetType()->Is(__RT_int32));
		ionassert(MMatterTest::__RF_IntField->GetType()->GetSize() == sizeof(int32));
		ionassert(!MMatterTest::__RF_IntField->GetType()->IsClass());
		ionassert(MMatterTest::__RF_IntField->GetName() == "IntField");
		ionassert(MMatterTest::__RF_IntField->GetOffset() == offsetof(MMatterTest, IntField));

		// Direct field interaction
		int32 value0 = MMatterTest::__RF_IntField->GetValueDirect<int32>(object);
		ionassert(value0 == object->IntField);
		MMatterTest::__RF_IntField->SetValueDirect(object, (int32)69);
		ionassert(object->IntField == 69);

		// Indirect field interaction
		MValuePtr mValue0 = MMatterTest::__RF_IntField->GetValueEx(object);
		ionassert(mValue0->As<int32>() == object->IntField);
		MMatterTest::__RF_IntField->SetValueEx(object, MValue::Create(69i32));
		ionassert(object->IntField == 69);

		// MReference
		MReferencePtr mRef0 = MMatterTest::__RF_IntField->GetReferenceEx(object);
		ionassert(mRef0->As<int32>() == object->IntField);
		mRef0->SetDirect(50);
		ionassert(object->IntField == 50);
		mRef0->Set(80);
		ionassert(object->IntField == 80);
		int32& mRef1 = MMatterTest::__RF_IntField->GetReference<int32>(object);
		ionassert(mRef1 == mRef0->As<int32>());
		mRef1 = 2323;
		ionassert(object->IntField == 2323);

		ionassert(MMatterTest::__RF_IntField2->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RF_IntField2->GetType() == __RT_int64);
		ionassert(MMatterTest::__RF_IntField2->GetType()->Is(__RT_int64));
		ionassert(MMatterTest::__RF_IntField2->GetType()->GetSize() == sizeof(int64));
		ionassert(!MMatterTest::__RF_IntField2->GetType()->IsClass());
		ionassert(MMatterTest::__RF_IntField2->GetName() == "IntField2");
		ionassert(MMatterTest::__RF_IntField2->GetOffset() == offsetof(MMatterTest, IntField2));

		ionassert(MMatterTest::__RF_MObjectField->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RF_MObjectField->GetType() == MObject::StaticClass());
		ionassert(MMatterTest::__RF_MObjectField->GetType()->Is(MObject::StaticClass()));
		ionassert(MMatterTest::__RF_MObjectField->GetType()->GetSize() == sizeof(MObject));
		ionassert(MMatterTest::__RF_MObjectField->GetType()->IsClass());
		ionassert(MMatterTest::__RF_MObjectField->GetName() == "MObjectField");
		ionassert(MMatterTest::__RF_MObjectField->GetOffset() == offsetof(MMatterTest, MObjectField));

		TObjectPtr<MMatterTest> object2 = MObject::New<MMatterTest>();
		MMatterTest::__RF_MObjectField->SetValueDirect(object, object2);
		ionassert(object->MObjectField == object2);
		ionassert(object->MObjectField == MMatterTest::__RF_MObjectField->GetValueDirect<MObjectPtr>(object));

		// Enum fields

		MMatterTest::__RF_EnumField->SetValueDirect(object, EMatterEnum::Value1);
		ionassert(object->EnumField == EMatterEnum::Value1);
		EMatterEnum enumRet0 = MMatterTest::__RF_EnumField->GetValueDirect<EMatterEnum>(object);
		ionassert(enumRet0 == EMatterEnum::Value1);

		// Array fields

		ionassert(object->ArrayField.size() == 0);
		MMatterTest::__RF_ArrayField->SetValue(object, TArray<int32> { 6, 4 });
		ionassert(object->ArrayField.size() == 2);
		ionassert(object->ArrayField[0] == 6);
		ionassert(object->ArrayField[1] == 4);
		TArray<int32> retArray0 = MMatterTest::__RF_ArrayField->GetValue<TArray<int32>>(object);
		ionassert(retArray0.size() == 2);
		ionassert(retArray0[0] == 6);
		ionassert(retArray0[1] == 4);

		// HashMap fields

		ionassert(object->HashMapField.size() == 0);
		MMatterTest::__RF_HashMapField->SetValue(object, THashMap<int32, String> { { 6, "Six" }, { 4, "Four" } });
		ionassert(object->HashMapField.size() == 2);
		ionassert(object->HashMapField.find(6) != object->HashMapField.end());
		ionassert(object->HashMapField.at(6) == "Six");
		ionassert(object->HashMapField.find(4) != object->HashMapField.end());
		ionassert(object->HashMapField.at(4) == "Four");
		THashMap<int32, String> retMap0 = MMatterTest::__RF_HashMapField->GetValue<THashMap<int32, String>>(object);
		ionassert(retMap0.size() == 2);
		ionassert(retMap0.find(6) != retMap0.end());
		ionassert(retMap0.at(6) == "Six");
		ionassert(retMap0.find(4) != retMap0.end());
		ionassert(retMap0.at(4) == "Four");

		// Methods

		ionassert(MMatterTest::__RM_Void_VoidMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_Void_VoidMethod->GetReturnType().PlainType == __RT_void);
		ionassert(MMatterTest::__RM_Void_VoidMethod->GetReturnType().PlainType->Is(__RT_void));
		ionassert(MMatterTest::__RM_Void_VoidMethod->GetParameterTypes().empty());
		ionassert(MMatterTest::__RM_Void_VoidMethod->GetName() == "Void_VoidMethod");

		MMatterTest::__RM_Void_VoidMethod->InvokeEx(object);
		MMatterTest::__RM_Void_VoidMethod->Invoke(object);

		ionassert(MMatterTest::__RM_Void_ConstRefIntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_Void_ConstRefIntMethod->GetReturnType().PlainType == __RT_void);
		ionassert(MMatterTest::__RM_Void_ConstRefIntMethod->GetReturnType().PlainType->Is(__RT_void));
		ionassert(MMatterTest::__RM_Void_ConstRefIntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::__RM_Void_ConstRefIntMethod->GetParameterTypes()[0].PlainType->Is<int32>());
		ionassert(MMatterTest::__RM_Void_ConstRefIntMethod->GetName() == "Void_ConstRefIntMethod");

		MMatterTest::__RM_Void_ConstRefIntMethod->Invoke(object, 69i32);

		ionassert(MMatterTest::__RM_Int_VoidMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_Int_VoidMethod->GetReturnType().PlainType == __RT_int32);
		ionassert(MMatterTest::__RM_Int_VoidMethod->GetReturnType().PlainType->Is(__RT_int32));
		ionassert(MMatterTest::__RM_Int_VoidMethod->GetParameterTypes().empty());
		ionassert(MMatterTest::__RM_Int_VoidMethod->GetName() == "Int_VoidMethod");

		MValuePtr intRet0 = MMatterTest::__RM_Int_VoidMethod->InvokeEx(object);
		ionassert(intRet0->As<int32>() == 420);

		int32 intRet0x = MMatterTest::__RM_Int_VoidMethod->Invoke<int32>(object);
		ionassert(intRet0x == 420);

		ionassert(MMatterTest::__RM_Int_IntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_Int_IntMethod->GetReturnType().PlainType == __RT_int32);
		ionassert(MMatterTest::__RM_Int_IntMethod->GetReturnType().PlainType->Is(__RT_int32));
		ionassert(MMatterTest::__RM_Int_IntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::__RM_Int_IntMethod->GetParameterTypes()[0].PlainType == __RT_int32);
		ionassert(MMatterTest::__RM_Int_IntMethod->GetParameterTypes()[0].PlainType->Is(__RT_int32));
		ionassert(MMatterTest::__RM_Int_IntMethod->GetName() == "Int_IntMethod");

		MValuePtr intRet1 = MMatterTest::__RM_Int_IntMethod->InvokeEx(object, { MValue::Create(2137) });
		ionassert(intRet1->As<int32>() == 2137);

		int32 intRet1x = MMatterTest::__RM_Int_IntMethod->Invoke<int32>(object, 2137i32);
		ionassert(intRet1x == 2137);

		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetReturnType().PlainType == MMatterTest::StaticClass());
		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetReturnType().PlainType->Is(MMatterTest::StaticClass()));
		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetParameterTypes()[0].PlainType == __RT_int32);
		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetParameterTypes()[0].PlainType->Is(__RT_int32));
		ionassert(MMatterTest::__RM_MMatterTest_IntMethod->GetName() == "MMatterTest_IntMethod");

		MValuePtr mObjectRet0 = MMatterTest::__RM_MMatterTest_IntMethod->InvokeEx(object, { MValue::Create(2137) });
		ionassert(mObjectRet0->As<MObjectPtr>() == object);

		MObjectPtr mObjectRet0x = MMatterTest::__RM_MMatterTest_IntMethod->Invoke<MObjectPtr>(object, 2137i32);
		ionassert(mObjectRet0x == object);

		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetReturnType().PlainType == MObject::StaticClass());
		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetReturnType().PlainType->Is(MObject::StaticClass()));
		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetParameterTypes()[0].PlainType == MObject::StaticClass());
		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetParameterTypes()[0].PlainType->Is(MObject::StaticClass()));
		ionassert(MMatterTest::__RM_MObject_MObjectMethod->GetName() == "MObject_MObjectMethod");

		MValuePtr mObjectRet1 = MMatterTest::__RM_MObject_MObjectMethod->InvokeEx(object, { MValue::Create(object2) });
		ionassert(mObjectRet1->As<MObjectPtr>() == object2);

		MObjectPtr mObjectRet1x = MMatterTest::__RM_MObject_MObjectMethod->Invoke<MObjectPtr>(object, object2);
		ionassert(mObjectRet1x == object2);

		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetReturnType().PlainType == MObject::StaticClass());
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetReturnType().PlainType->Is(MObject::StaticClass()));
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetParameterTypes().size() == 2);
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetParameterTypes()[0].PlainType == MObject::StaticClass());
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetParameterTypes()[0].PlainType->Is(MObject::StaticClass()));
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetParameterTypes()[1].PlainType == __RT_int32);
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetParameterTypes()[1].PlainType->Is(__RT_int32));
		ionassert(MMatterTest::__RM_MObject_MObjectIntMethod->GetName() == "MObject_MObjectIntMethod");

		MValuePtr mObjectRet2 = MMatterTest::__RM_MObject_MObjectIntMethod->InvokeEx(object, { MValue::Create(object2), MValue::Create(69) });
		ionassert(mObjectRet2->As<MObjectPtr>() == object2);

		MObjectPtr mObjectRet2x = MMatterTest::__RM_MObject_MObjectIntMethod->Invoke<MObjectPtr>(object, object2, 69i32);
		ionassert(mObjectRet2x == object2);

		// String

		ionassert(MMatterTest::__RM_Void_StringMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_Void_StringMethod->GetReturnType().PlainType == __RT_void);
		ionassert(MMatterTest::__RM_Void_StringMethod->GetReturnType().PlainType->Is(__RT_void));
		ionassert(MMatterTest::__RM_Void_StringMethod->GetReturnType().PlainType->Is<void>());
		ionassert(MMatterTest::__RM_Void_StringMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::__RM_Void_StringMethod->GetParameterTypes()[0].PlainType == __RT_String);
		ionassert(MMatterTest::__RM_Void_StringMethod->GetParameterTypes()[0].PlainType->Is(__RT_String));
		ionassert(MMatterTest::__RM_Void_StringMethod->GetParameterTypes()[0].PlainType->Is<String>());
		ionassert(MMatterTest::__RM_Void_StringMethod->GetName() == "Void_StringMethod");

		MMatterTest::__RM_Void_StringMethod->Invoke(object, "Hello"s);

		ionassert(MMatterTest::__RM_String_VoidMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_String_VoidMethod->GetReturnType().PlainType == __RT_String);
		ionassert(MMatterTest::__RM_String_VoidMethod->GetReturnType().PlainType->Is(__RT_String));
		ionassert(MMatterTest::__RM_String_VoidMethod->GetReturnType().PlainType->Is<String>());
		ionassert(MMatterTest::__RM_String_VoidMethod->GetParameterTypes().size() == 0);
		ionassert(MMatterTest::__RM_String_VoidMethod->GetName() == "String_VoidMethod");

		String stringRet0 = MMatterTest::__RM_String_VoidMethod->Invoke<String>(object);
		ionassert(stringRet0 == object->Str420);

		// GUID

		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetClass()->Is<MMatterTest>());
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetReturnType().PlainType == __RT_GUID);
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetReturnType().PlainType->Is(__RT_GUID));
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetReturnType().PlainType->Is<GUID>());
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetParameterTypes().size() == 1);
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetParameterTypes()[0].PlainType == __RT_GUID);
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetParameterTypes()[0].PlainType->Is(__RT_GUID));
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetParameterTypes()[0].PlainType->Is<GUID>());
		ionassert(MMatterTest::__RM_GUID_GUIDMethod->GetName() == "GUID_GUIDMethod");

		GUID guid = GUID::FromString("d5e2f8b2-5727-4768-9a74-d182b92c1b6c").Unwrap();
		GUID guidRet0 = MMatterTest::__RM_GUID_GUIDMethod->Invoke<GUID>(object, guid);
		ionassert(guidRet0 == guid);

		// Array

		TArray<int32> arr { 5, 2, 8 };
		MReferencePtr arrRet0 = MMatterTest::__RM_Array_ArrayMethod->InvokeEx(object, TArray<MMethodTypeInstance> { MReference::CreateConst(arr) });
		TArray<int32>& arrRef0 = arrRet0->As<TArray<int32>>();
		ionassert(&arrRef0 == &arr);
		ionassert(arr.size() == arrRef0.size());
		ionassert([&] { for (int32 i = 0; i < arr.size(); ++i) if (arr[i] != arrRef0[i]) return false; return true; }());

		// Hash map

		THashMap<int32, String> map { { 1, "Test" }, { 10, "Test10" } };
		MReferencePtr mapRet0 = MMatterTest::__RM_HashMap_HashMapMethod->InvokeEx(object, TArray<MMethodTypeInstance> { MReference::CreateConst(map) });
		THashMap<int32, String>& mapRef0 = mapRet0->As<THashMap<int32, String>>();
		ionassert(&mapRef0 == &map);
		ionassert(map.size() == mapRef0.size());
		ionassert([&] { for (const auto& [k, v] : map) if (mapRef0.find(k) == mapRef0.end() || v != mapRef0.at(k)) return false; return true; }());

		// Iteration

		TArray<MField*> fields = c->GetFields();
		for (MField* field : fields)
		{
			MReflectionLogger.Debug("Field: {} {}::{}",
				field->GetType()->GetName(),
				field->GetClass()->GetName(),
				field->GetName());
		}

		TArray<MMethod*> methods = c->GetMethods();
		for (MMethod* method : methods)
		{
			MReflectionLogger.Debug("Method: {} {}::{}({})",
				method->GetReturnType().PlainType->GetName(),
				method->GetClass()->GetName(),
				method->GetName(),
				[method]
				{
					const TArray<MMethodType>& parameters = method->GetParameterTypes();
					TArray<String> parameterNames;
					parameterNames.reserve(parameters.size());
					std::transform(parameters.begin(), parameters.end(), std::back_inserter(parameterNames), [](const MMethodType& type)
					{
						return type.PlainType->GetName();
					});
					return JoinString(parameterNames, ", "s);
				}());
		}

		// Archive test

		XMLArchive testAr(EArchiveType::Saving);
		testAr.SeekRoot();
		testAr &= object;
		{
			File saveFile("MatterArchiveTest.xml");
			testAr.SaveToFile(saveFile);
		}

		//ionbreak();
	}

#pragma endregion

#pragma region Matter Composite test
	
	class MMatterComponent : public MObject
	{
		MCLASS(MMatterComponent)
		using Super = MObject;

		MMatterComponent()
		{
		}

		String Str = "TestString";
		MFIELD(Str)

		int32 Int = 69;
		MFIELD(Int)
	};
	
	class MMatterComposite : public MObject
	{
		MCLASS(MMatterComposite)
		using Super = MObject;

		MMatterComposite() :
			Component(MObject::ConstructDefault<MMatterComponent>())
		{
			Component->Int = 420;
			Component->Str = "From Composite";
		}

		TObjectPtr<MMatterComponent> Component;
		MFIELD(Component)

		using adfd = TRemoveObjectPtr<decltype(Component)>;
		String CompositeStr = "CompositeString";
		MFIELD(CompositeStr)
	};

	void MatterCompositeTest()
	{
		MClass* componentClass = MMatterComponent::__RT;
		MClass* compositeClass = MMatterComposite::__RT;

		TObjectPtr<MMatterComponent> component0 = MObject::New<MMatterComponent>();

		TObjectPtr<MMatterComposite> composite0 = MObject::New<MMatterComposite>();
		TObjectPtr<MMatterComposite> composite1 = MObject::New<MMatterComposite>();

		ionassert(component0->GetClass()->Is(componentClass));
		ionassert(composite0->GetClass()->Is(compositeClass));
		ionassert(composite1->GetClass()->Is(compositeClass));

		ionassert(MMatterComposite::__RF_Component->GetType()->Is(componentClass));

		ionassert(MMatterComposite::__RT->GetClassDefaultObjectTyped<MMatterComposite>()->Component != MMatterComponent::__RT->GetClassDefaultObject());

		ionassert(MMatterComposite::__RT->GetClassDefaultObjectTyped<MMatterComposite>()->Component->Int == 420);
		ionassert(composite0->Component->Int == MMatterComposite::__RT->GetClassDefaultObjectTyped<MMatterComposite>()->Component->Int);
		ionassert(composite0->Component->Int == 420);
		ionassert(composite0->Component->Str == "From Composite");
		ionassert(MMatterComponent::__RT->GetClassDefaultObjectTyped<MMatterComponent>()->Int == 69);
		ionassert(component0->Int == 69);
		ionassert(component0->Str == "TestString");
		ionassert(component0->Int == MMatterComponent::__RT->GetClassDefaultObjectTyped<MMatterComponent>()->Int);

		composite0->Component->Int = 10;
		composite1->Component->Int = 20;
		ionassert(composite0->Component->Int == 10);
		ionassert(composite1->Component->Int == 20);
		ionassert(composite0->Component->Int != composite1->Component->Int);
		ionassert(composite0->Component != composite1->Component);
	}

	class MMatterTickTest : public MObject
	{
		MCLASS(MMatterTickTest)
		using Super = MObject;

	public:
		MMatterTickTest()
		{
			SetTickEnabled(true);
		}

		virtual void OnCreate() override
		{
			MReflectionLogger.Trace("OnCreate {}.", GetName());
		}

		virtual void OnDestroy() override
		{
			MReflectionLogger.Trace("OnDestroy {}.", GetName());
		}

		virtual void Tick(float deltaTime) override
		{
			MReflectionLogger.Trace("Tick {} {:.2f}ms", GetName(), deltaTime * 1000.0f);
		}
	};

	void MatterTickTest()
	{
		static TObjectPtr<MMatterTickTest> tickTest = MObject::New<MMatterTickTest>();
	}

#pragma endregion

	void MatterTest()
	{
		MatterClassTest();
		MatterCompositeTest();
		MatterTickTest();
	}
}
