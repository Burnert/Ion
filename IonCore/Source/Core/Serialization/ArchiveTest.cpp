#include "Core/CorePCH.h"

#include "Archive.h"
#include "BinaryArchive.h"
#include "Core/GUID/GUID.h"
#include "Core/Container/Tree.h"
#include "Core/Container/TreeSerializer.h"

namespace Ion::Test
{
	void ArchiveTest()
	{
		Archive& archive = *new BinaryArchive(EArchiveType::Saving);

		if (archive.IsLoading())
		{
			File loadFile("ArchiveTest.dat");
			archive.LoadFromFile(loadFile);
		}

		// ----------------------------------------------------

		// GUID test

		GUID guidTest = archive.IsSaving() ? GUID::FromString("54a6f55c-feaf-4aa9-87cd-cc9b487c31ef").Unwrap() : GUID::Zero;
		archive << guidTest;
		ionassert(guidTest == GUID::FromString("54a6f55c-feaf-4aa9-87cd-cc9b487c31ef").Unwrap());

		// Tree test

		{
			TTreeNodeFactory<uint64> factory;
			TFastTreeNode<uint64>& tree = factory.Create(archive.IsSaving() ? 7 : 0);

			if (archive.IsSaving())
			{
				tree.Insert(factory.Create(10));
				auto& node5 = tree.Insert(factory.Create(5));

				node5.Insert(factory.Create(2));
				node5.Insert(factory.Create(8));
			}

			archive << TTreeSerializer(tree, factory);
			ionassert(tree.GetChildrenSize() == 2);
			ionassert(tree.GetChildren()[1]->GetChildrenSize() == 2);
			ionassert(tree.GetChildren()[1]->GetChildren()[0]->Get() == 2);
		}

		// String test

		String stringTest1 = archive.IsSaving() ? "ABCDEFG_STRTEST" : EmptyString;
		archive << stringTest1;
		ionassert(stringTest1 == "ABCDEFG_STRTEST");
		String stringTest2 = archive.IsSaving() ? "NextString_____" : EmptyString;
		archive << stringTest2;
		ionassert(stringTest2 == "NextString_____");
		String stringTest3 = archive.IsSaving() ? "  String3      " : EmptyString;
		archive << stringTest3;
		ionassert(stringTest3 == "  String3      ");
		const char utfString[] = u8"u8 简体中文";
		String stringTest4 = archive.IsSaving() ? utfString : EmptyString;
		archive << stringTest4;
		ionassert(stringTest4 == utfString);

		// TArray test

		TFixedArray<int32, 10> arrayTest_base {
			323, 439810, 590211, 19, 69999192,
			53589823, 328981, 2, 0, 5989391
		};
		TArray<int32> arrayTest;
		if (archive.IsSaving())
		{
			std::copy(arrayTest_base.begin(), arrayTest_base.end(), std::back_inserter(arrayTest));
		}
		archive << arrayTest;
		ionassert([&]
		{
			if (arrayTest.size() != arrayTest.size())
				return false;

			for (int32 i = 0; i < arrayTest.size(); ++i)
			{
				if (arrayTest[i] != arrayTest_base[i])
					return false;
			}
			return true;
		}());

		// ----------------------------------------------------

		if (archive.IsSaving())
		{
			File saveFile("ArchiveTest.dat");
			archive.SaveToFile(saveFile);
		}

		//ionbreak();

		delete &archive;
	}
}
