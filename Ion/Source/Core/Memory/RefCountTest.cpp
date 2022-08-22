#include "IonPCH.h"

#include "RefCount.h"

//#if ION_TEST

namespace Ion
{
	class RefTest : public RefCountable
	{
	public:
		String Text;
		RefTest(const String& text) : Text(text) { }
	};

	class RefTest2 : public RefTest
	{
	public:
		String Text2;
		RefTest2(const String& text, const String& text2) : RefTest(text), Text2(text2) { }
	};
	
	class RefDelTest : public RefCountable
	{
	public:
		uint64 Number = 0x1234567890;
	};

	int RefCountTest()
	{
		// Default constructor
		TRef<RefTest> ref0;
		ionassert(!ref0);
		ionassert(ref0.GetRaw() == nullptr);

		// Null constructor
		TRef<RefTest> ref1 = nullptr;
		ionassert(!ref1);
		ionassert(ref1.GetRaw() == nullptr);

		// Null assignment
		ref0 = nullptr;
		ionassert(!ref0);
		ionassert(ref0.GetRaw() == nullptr);

		// Pointer assignment
		RefTest* pRefTest;
		TRef<RefTest> ref2 = pRefTest = new RefTest("Text");
		ionassert(ref2);
		ionassert(ref2.GetRaw() != nullptr);
		ionassert(ref2.GetRaw() == pRefTest);
		ionassert(ref2->Text == "Text");
		ionassert(ref2->Text == pRefTest->Text);
		ionassert((*ref2).Text == "Text");

		// MakeRef
		TRef<RefTest> ref3 = MakeRef<RefTest>("Text2");
		ionassert(ref3);
		ionassert(ref3->Text == "Text2");
		ionassert(ref3.GetRefCount() == 1);
		ionassert(ref3.GetRefCount() == ref3->GetRefCount());

		// Copy constructor
		TRef<RefTest> ref4 = ref3;
		ionassert(ref4);
		ionassert(ref4->Text == ref3->Text);
		ionassert(ref4.GetRaw() == ref3.GetRaw());
		ionassert(ref4 == ref3);
		ionassert(ref4.GetRefCount() == 2);

		// Move constructor
		String text3 = ref3->Text;
		RefTest* pRefTest3 = ref3.GetRaw();
		TRef<RefTest> ref5 = Move(ref3);
		ionassert(ref5);
		ionassert(ref5->Text == text3);
		ionassert(ref5.GetRaw() == pRefTest3);
		ionassert(ref5 != ref3);
		ionassert(ref5.GetRefCount() == 2);

		// Copy assignment
		ref0 = ref4;
		ionassert(ref0);
		ionassert(ref4);
		ionassert(ref0.GetRaw() == ref4.GetRaw());
		ionassert(ref0 == ref4);
		ionassert(ref0.GetRefCount() == 3);
		ionassert(ref0->Text == ref4->Text);
		ref0 = ref2;
		ionassert(ref4.GetRefCount() == 2);
		ionassert(ref2.GetRefCount() == 2);
		ionassert(ref0 == ref2);
		ionassert(ref0 != ref4);
		ref0 = ref4;

		// Move assignment
		ref1 = Move(ref4);
		ionassert(ref1);
		ionassert(!ref4);
		ionassert(ref1.GetRaw() == ref0.GetRaw());
		ionassert(ref1.GetRaw() != ref4.GetRaw());
		ionassert(ref1 == ref0);
		ionassert(ref1 != ref4);
		ionassert(ref1.GetRefCount() == 3);
		ionassert(ref1.GetRefCount() == ref0.GetRefCount());
		ionassert(ref1.GetRefCount() != ref4.GetRefCount());
		ionassert(ref1->Text == ref0->Text);
		TRef<RefTest> ref2Backup = ref2;
		ref1 = Move(ref2);
		ionassert(ref0.GetRefCount() == 2);
		ionassert(ref1.GetRefCount() == ref2Backup.GetRefCount());
		ref1 = ref0;
		ref2 = Move(ref2Backup);

		// Swap
		TRef<RefTest> ref6 = MakeRef<RefTest>("TextSwap");
		ionassert(ref6.GetRefCount() == 1);
		ionassert(ref0.GetRefCount() == 3);
		ref0.Swap(ref6);
		ionassert(ref6.GetRefCount() == 3);
		ionassert(ref0.GetRefCount() == 1);
		ionassert(ref6->Text == "Text2");
		ionassert(ref0->Text == "TextSwap");

		// Polymorphic

		// Copy
		TRef<RefTest2> rt2_0 = MakeRef<RefTest2>("Text1", "Text2");
		TRef<RefTest> rt1_0 = rt2_0;
		ionassert(rt2_0 == rt1_0);
		ionassert(rt2_0.GetRaw() == rt1_0.GetRaw());
		ionassert(rt2_0.GetRefCount() == rt1_0.GetRefCount());
		ionassert(rt2_0.GetRefCount() == 2);
		ionassert(rt1_0->Text == rt2_0->Text);
		ionassert(rt1_0->Text == "Text1");

		TRef<RefTest> rt1_1;
		rt1_1 = rt2_0;
		ionassert(rt1_1 == rt2_0);
		ionassert(rt1_1.GetRaw() == rt2_0.GetRaw());
		ionassert(rt1_1.GetRefCount() == rt2_0.GetRefCount());
		ionassert(rt2_0.GetRefCount() == 3);

		// Move
		TRef<RefTest2> rt2_1 = rt2_0;
		ionassert(rt2_1.GetRefCount() == 4);
		TRef<RefTest> rt1_2 = Move(rt2_1);
		ionassert(!rt2_1);
		ionassert(rt1_2 == rt2_0);
		ionassert(rt1_2 != rt2_1);
		ionassert(rt1_2.GetRefCount() == 4);
		ionassert(rt1_2.GetRefCount() != rt2_1.GetRefCount());

		TRef<RefTest> rt1_3;
		ionassert(rt2_0.GetRefCount() == 4);
		rt2_1 = rt2_0;
		ionassert(rt2_0.GetRefCount() == 5);
		rt1_3 = Move(rt2_1);
		ionassert(rt2_0.GetRefCount() == 5);
		ionassert(!rt2_1);
		ionassert(rt1_3);
		ionassert(rt1_3 == rt2_0);
		ionassert(rt1_3 != rt2_1);
		ionassert(rt1_3.GetRefCount() == 5);
		ionassert(rt1_3.GetRefCount() != rt2_1.GetRefCount());

		// RefCast
		TRef<RefTest> poly0 = MakeRef<RefTest2>("Text1", "Text2");
		TRef<RefTest2> poly1 = RefCast<RefTest2>(poly0);
		ionassert(poly0 == poly1);
		poly1 = RefCast<RefTest2>(Move(poly0));
		ionassert(poly0 != poly1);
		ionassert(!poly0);

		// DynamicRefCast
		TRef<RefTest> poly2 = MakeRef<RefTest>("Text2");
		poly0 = DynamicRefCast<RefTest>(poly2);
		poly1 = DynamicRefCast<RefTest2>(poly2);
		ionassert(poly0);
		ionassert(!poly1);
		poly0 = DynamicRefCast<RefTest>(Move(poly2));
		ionassert(!poly2);
		poly2 = poly0;
		poly1 = DynamicRefCast<RefTest2>(Move(poly2));
		ionassert(poly2);
		ionassert(poly0);
		ionassert(!poly1);

		// Deletion

		TRef<RefDelTest> del0 = MakeRef<RefDelTest>();
		RefDelTest* refTestRef = del0.GetRaw();
		ionassert(refTestRef->Number == 0x1234567890);
		// Get the offset of the field to the object start
		ptrdiff_t fieldOffset = (uint64*)&refTestRef->Number - (uint64*)refTestRef;
		ionassert(*((uint64*)refTestRef + fieldOffset) == refTestRef->Number);
		{
			TRef<RefDelTest> del1 = Move(del0);
			ionassert(!del0);
			uint64 number = *((uint64*)refTestRef + fieldOffset);
			ionassert(number == 0x1234567890);
		}
		uint64 number = *((uint64*)refTestRef + fieldOffset);
		ionassert(number != 0x1234567890); // << probably works on debug only

		return 0;
	}
}

//#endif
