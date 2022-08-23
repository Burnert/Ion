#include "IonPCH.h"

#include "RefCount.h"

//#if ION_TEST

namespace Ion
{
#pragma region Intrusive

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
		ionassert(ref0.Raw() == nullptr);

		// Null constructor
		TRef<RefTest> ref1 = nullptr;
		ionassert(!ref1);
		ionassert(ref1.Raw() == nullptr);

		// Null assignment
		ref0 = nullptr;
		ionassert(!ref0);
		ionassert(ref0.Raw() == nullptr);

		// Pointer construction
		RefTest* pRefTest;
		TRef<RefTest> ref2 = pRefTest = new RefTest("Text");
		ionassert(ref2);
		ionassert(ref2.Raw() != nullptr);
		ionassert(ref2.Raw() == pRefTest);
		ionassert(ref2->Text == "Text");
		ionassert(ref2->Text == pRefTest->Text);
		ionassert((*ref2).Text == "Text");

		// MakeRef
		TRef<RefTest> ref3 = MakeRef<RefTest>("Text2");
		ionassert(ref3);
		ionassert(ref3->Text == "Text2");
		ionassert(ref3.RefCount() == 1);
		ionassert(ref3.RefCount() == ref3->RefCount());

		// Copy constructor
		TRef<RefTest> ref4 = ref3;
		ionassert(ref4);
		ionassert(ref4->Text == ref3->Text);
		ionassert(ref4.Raw() == ref3.Raw());
		ionassert(ref4 == ref3);
		ionassert(ref4.RefCount() == 2);

		// Move constructor
		String text3 = ref3->Text;
		RefTest* pRefTest3 = ref3.Raw();
		TRef<RefTest> ref5 = Move(ref3);
		ionassert(ref5);
		ionassert(ref5->Text == text3);
		ionassert(ref5.Raw() == pRefTest3);
		ionassert(ref5 != ref3);
		ionassert(ref5.RefCount() == 2);

		// Copy assignment
		ref0 = ref4;
		ionassert(ref0);
		ionassert(ref4);
		ionassert(ref0.Raw() == ref4.Raw());
		ionassert(ref0 == ref4);
		ionassert(ref0.RefCount() == 3);
		ionassert(ref0->Text == ref4->Text);
		ref0 = ref2;
		ionassert(ref4.RefCount() == 2);
		ionassert(ref2.RefCount() == 2);
		ionassert(ref0 == ref2);
		ionassert(ref0 != ref4);
		ref0 = ref4;

		// Move assignment
		ref1 = Move(ref4);
		ionassert(ref1);
		ionassert(!ref4);
		ionassert(ref1.Raw() == ref0.Raw());
		ionassert(ref1.Raw() != ref4.Raw());
		ionassert(ref1 == ref0);
		ionassert(ref1 != ref4);
		ionassert(ref1.RefCount() == 3);
		ionassert(ref1.RefCount() == ref0.RefCount());
		ionassert(ref1.RefCount() != ref4.RefCount());
		ionassert(ref1->Text == ref0->Text);
		TRef<RefTest> ref2Backup = ref2;
		ref1 = Move(ref2);
		ionassert(ref0.RefCount() == 2);
		ionassert(ref1.RefCount() == ref2Backup.RefCount());
		ref1 = ref0;
		ref2 = Move(ref2Backup);

		// Swap
		TRef<RefTest> ref6 = MakeRef<RefTest>("TextSwap");
		ionassert(ref6.RefCount() == 1);
		ionassert(ref0.RefCount() == 3);
		ref0.Swap(ref6);
		ionassert(ref6.RefCount() == 3);
		ionassert(ref0.RefCount() == 1);
		ionassert(ref6->Text == "Text2");
		ionassert(ref0->Text == "TextSwap");

		// Polymorphic

		// Copy
		TRef<RefTest2> rt2_0 = MakeRef<RefTest2>("Text1", "Text2");
		TRef<RefTest> rt1_0 = rt2_0;
		ionassert(rt2_0 == rt1_0);
		ionassert(rt2_0.Raw() == rt1_0.Raw());
		ionassert(rt2_0.RefCount() == rt1_0.RefCount());
		ionassert(rt2_0.RefCount() == 2);
		ionassert(rt1_0->Text == rt2_0->Text);
		ionassert(rt1_0->Text == "Text1");

		TRef<RefTest> rt1_1;
		rt1_1 = rt2_0;
		ionassert(rt1_1 == rt2_0);
		ionassert(rt1_1.Raw() == rt2_0.Raw());
		ionassert(rt1_1.RefCount() == rt2_0.RefCount());
		ionassert(rt2_0.RefCount() == 3);

		// Move
		TRef<RefTest2> rt2_1 = rt2_0;
		ionassert(rt2_1.RefCount() == 4);
		TRef<RefTest> rt1_2 = Move(rt2_1);
		ionassert(!rt2_1);
		ionassert(rt1_2 == rt2_0);
		ionassert(rt1_2 != rt2_1);
		ionassert(rt1_2.RefCount() == 4);
		ionassert(rt1_2.RefCount() != rt2_1.RefCount());

		TRef<RefTest> rt1_3;
		ionassert(rt2_0.RefCount() == 4);
		rt2_1 = rt2_0;
		ionassert(rt2_0.RefCount() == 5);
		rt1_3 = Move(rt2_1);
		ionassert(rt2_0.RefCount() == 5);
		ionassert(!rt2_1);
		ionassert(rt1_3);
		ionassert(rt1_3 == rt2_0);
		ionassert(rt1_3 != rt2_1);
		ionassert(rt1_3.RefCount() == 5);
		ionassert(rt1_3.RefCount() != rt2_1.RefCount());

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
		RefDelTest* refTestRef = del0.Raw();
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

#pragma endregion

#pragma region Non-Intrusive

	class PtrTest
	{
	public:
		String Text;
		PtrTest(const String& text) : Text(text) { }

		virtual ~PtrTest() {}
	};

	class PtrTest2 : public PtrTest
	{
	public:
		String Text2;
		PtrTest2(const String& text, const String& text2) : PtrTest(text), Text2(text2) { }
	};

	int RefCountPtrTest()
	{
		// Default constructor
		TSharedPtr<PtrTest> ptr0;
		ionassert(!ptr0);
		ionassert(!ptr0.Raw());
		//ionassert(ptr0 == nullptr);

		// Null constructor
		TSharedPtr<PtrTest> ptr1 = nullptr;
		ionassert(!ptr1);
		ionassert(!ptr1.Raw());
		ionassert(ptr0 == ptr1);

		// Null assignment
		ptr0 = nullptr;
		ionassert(!ptr0);
		ionassert(ptr0.Raw() == nullptr);

		// Pointer construction
		PtrTest* pPtrTest = new PtrTest("Text1");
		TSharedPtr<PtrTest> ptr2 = MakeSharedFrom(pPtrTest);
		ionassert(ptr2);
		ionassert(ptr2->Text == "Text1");
		ionassert((*ptr2).Text == "Text1");
		ionassert(ptr2.Raw()->Text == ptr2->Text);
		ionassert(ptr2.Raw() != nullptr);
		ionassert(ptr2.Raw() == pPtrTest);
		ionassert(ptr2->Text == "Text1");
		ionassert(ptr2->Text == pPtrTest->Text);
		ionassert((*ptr2).Text == "Text1");

		// MakeShared
		TSharedPtr<PtrTest> ptr3 = MakeSharedFrom(new PtrTest("Text2"));
		ionassert(ptr3);
		ionassert(ptr3->Text == "Text2");
		ionassert(ptr3.RefCount() == 1);

		// Copy constructor
		TSharedPtr<PtrTest> ptr4 = ptr3;
		ionassert(ptr4);
		ionassert(ptr4->Text == ptr3->Text);
		ionassert(ptr4.Raw() == ptr3.Raw());
		ionassert(ptr4 == ptr3);
		ionassert(ptr4.RefCount() == 2);

		// Move constructor
		String text3 = ptr3->Text;
		PtrTest* pPtrTest3 = ptr3.Raw();
		TSharedPtr<PtrTest> ptr5 = Move(ptr3);
		ionassert(ptr5);
		ionassert(ptr5->Text == text3);
		ionassert(ptr5.Raw() == pPtrTest3);
		ionassert(ptr5 != ptr3);
		ionassert(ptr5.RefCount() == 2);

		// Copy assignment
		ptr0 = ptr4;
		ionassert(ptr0);
		ionassert(ptr4);
		ionassert(ptr0.Raw() == ptr4.Raw());
		ionassert(ptr0 == ptr4);
		ionassert(ptr0.RefCount() == 3);
		ionassert(ptr0->Text == ptr4->Text);
		ptr0 = ptr2;
		ionassert(ptr4.RefCount() == 2);
		ionassert(ptr2.RefCount() == 2);
		ionassert(ptr0 == ptr2);
		ionassert(ptr0 != ptr4);
		ptr0 = ptr4;

		// Move assignment
		ptr1 = Move(ptr4);
		ionassert(ptr1);
		ionassert(!ptr4);
		ionassert(ptr1.Raw() == ptr0.Raw());
		ionassert(ptr1.Raw() != ptr4.Raw());
		ionassert(ptr1 == ptr0);
		ionassert(ptr1 != ptr4);
		ionassert(ptr1.RefCount() == 3);
		ionassert(ptr1.RefCount() == ptr0.RefCount());
		ionassert(ptr1.RefCount() != ptr4.RefCount());
		ionassert(ptr1->Text == ptr0->Text);
		TSharedPtr<PtrTest> ptr2Backup = ptr2;
		ptr1 = Move(ptr2);
		ionassert(ptr0.RefCount() == 2);
		ionassert(ptr1.RefCount() == ptr2Backup.RefCount());
		ptr1 = ptr0;
		ptr2 = Move(ptr2Backup);

		// Polymorphic

		// Copy
		TSharedPtr<PtrTest2> pt2_0 = MakeSharedFrom(new PtrTest2("Text1", "Text2"));
		TSharedPtr<PtrTest> pt1_0 = pt2_0;
		ionassert(pt2_0 == pt1_0);
		ionassert(pt2_0.Raw() == pt1_0.Raw());
		ionassert(pt2_0.RefCount() == pt1_0.RefCount());
		ionassert(pt2_0.RefCount() == 2);
		ionassert(pt1_0->Text == pt2_0->Text);
		ionassert(pt1_0->Text == "Text1");

		TSharedPtr<PtrTest> pt1_1;
		pt1_1 = pt2_0;
		ionassert(pt1_1 == pt2_0);
		ionassert(pt1_1.Raw() == pt2_0.Raw());
		ionassert(pt1_1.RefCount() == pt2_0.RefCount());
		ionassert(pt2_0.RefCount() == 3);

		// Move
		TSharedPtr<PtrTest> pt2_1 = pt2_0;
		ionassert(pt2_1.RefCount() == 4);
		TSharedPtr<PtrTest> pt1_2 = Move(pt2_1);
		ionassert(!pt2_1);
		ionassert(pt1_2 == pt2_0);
		ionassert(pt1_2 != pt2_1);
		ionassert(pt1_2.RefCount() == 4);
		ionassert(pt1_2.RefCount() != pt2_1.RefCount());

		TSharedPtr<PtrTest> pt1_3;
		ionassert(pt2_0.RefCount() == 4);
		pt2_1 = pt2_0;
		ionassert(pt2_0.RefCount() == 5);
		pt1_3 = Move(pt2_1);
		ionassert(pt2_0.RefCount() == 5);
		ionassert(!pt2_1);
		ionassert(pt1_3);
		ionassert(pt1_3 == pt2_0);
		ionassert(pt1_3 != pt2_1);
		ionassert(pt1_3.RefCount() == 5);
		ionassert(pt1_3.RefCount() != pt2_1.RefCount());

		// PtrCast
		TSharedPtr<PtrTest> poly0 = MakeSharedFrom(new PtrTest2("Text1", "Text2"));
		TSharedPtr<PtrTest2> poly1 = PtrCast<PtrTest2>(poly0);
		ionassert(poly0 == poly1);
		poly1 = PtrCast<PtrTest2>(Move(poly0));
		ionassert(poly0 != poly1);
		ionassert(!poly0);

		// DynamicPtrCast
		TSharedPtr<PtrTest> poly2 = MakeSharedFrom(new PtrTest("Text2"));
		poly0 = DynamicPtrCast<PtrTest>(poly2);
		poly1 = DynamicPtrCast<PtrTest2>(poly2);
		ionassert(poly0);
		ionassert(!poly1);
		poly0 = DynamicPtrCast<PtrTest>(Move(poly2));
		ionassert(!poly2);
		poly2 = poly0;
		poly1 = DynamicPtrCast<PtrTest2>(Move(poly2));
		ionassert(poly2);
		ionassert(poly0);
		ionassert(!poly1);

		// TWeakPtr

		TSharedPtr<PtrTest> sptr0 = MakeSharedFrom(new PtrTest("WeakTest"));

		TWeakPtr<PtrTest> wptr0;
		ionassert(!wptr0);

		TWeakPtr<PtrTest> wptr1 = nullptr;
		ionassert(!wptr1);
		ionassert(wptr0 == wptr1);

		TWeakPtr<PtrTest> wptr2 = sptr0;
		ionassert(wptr2);

		return 0;
	}

#pragma endregion
}

//#endif
