#include "Core/CorePCH.h"

#include "Archive.h"
#include "BinaryArchive.h"
#include "YAMLArchive.h"
#include "Core/GUID/GUID.h"
#include "Core/Container/Tree.h"
#include "Core/Container/TreeSerializer.h"

namespace Ion::Test
{
	void GenericArchiveTest(Archive& archive)
	{
		// ----------------------------------------------------

		ArchiveNode root = archive.EnterRootNode();

		// GUID test

		ArchiveNode nodeGuid = archive.EnterNode(root, "Guid", EArchiveNodeType::Value);
		GUID guidTest = archive.IsSaving() ? GUID::FromString("54a6f55c-feaf-4aa9-87cd-cc9b487c31ef").Unwrap() : GUID::Zero;
		nodeGuid << guidTest;
		ionassert(guidTest == GUID::FromString("54a6f55c-feaf-4aa9-87cd-cc9b487c31ef").Unwrap());

		// String test

		ArchiveNode nodeS1 = archive.EnterNode(root, "S1", EArchiveNodeType::Value);
		String stringTest1 = archive.IsSaving() ? "ABCDEFG_STRTEST" : EmptyString;
		nodeS1 << stringTest1;
		ionassert(stringTest1 == "ABCDEFG_STRTEST");

		ArchiveNode nodeS2 = archive.EnterNode(root, "S2", EArchiveNodeType::Value);
		String stringTest2 = archive.IsSaving() ? "NextString_____" : EmptyString;
		nodeS2 << stringTest2;
		ionassert(stringTest2 == "NextString_____");

		ArchiveNode nodeS3 = archive.EnterNode(root, "S3", EArchiveNodeType::Value);
		String stringTest3 = archive.IsSaving() ? "  String3      " : EmptyString;
		nodeS3 << stringTest3;
		ionassert(stringTest3 == "  String3      ");

		ArchiveNode nodeS4 = archive.EnterNode(root, "S4", EArchiveNodeType::Value);
		const char utfString[] = u8"u8 简体中文";
		String stringTest4 = archive.IsSaving() ? utfString : EmptyString;
		nodeS4 << stringTest4;
		ionassert(stringTest4 == utfString);

		// TArray test

		ArchiveNode nodeArray = archive.EnterNode(root, "Array", EArchiveNodeType::Seq);
		TFixedArray<int32, 10> arrayTest_base {
			323, 439810, 590211, 19, 69999192,
			53589823, 328981, 2, 0, 5989391
		};
		TArray<int32> arrayTest;
		arrayTest.reserve(arrayTest_base.size());
		if (archive.IsSaving())
		{
			std::copy(arrayTest_base.begin(), arrayTest_base.end(), std::back_inserter(arrayTest));
		}
		nodeArray << arrayTest;
		ionassert(arrayTest.size() == arrayTest_base.size());
		ionassert([&]
		{
			for (int32 i = 0; i < arrayTest.size(); ++i)
			{
				if (arrayTest[i] != arrayTest_base[i])
					return false;
			}
			return true;
		}());

		ON_YAML_AR(archive) return;

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
	}

	void TestArchives(EArchiveType type)
	{
		struct ArchiveDesc
		{
			TSharedPtr<Archive> Ar;
			String Ext;
		};

		TArray<ArchiveDesc> archives = {
			{ MakeShared<BinaryArchive>(type), "dat" },
			{ MakeShared<YAMLArchive>(type), "yaml" }
		};

		for (const ArchiveDesc& desc : archives)
		{
			if (type == EArchiveType::Loading)
			{
				File loadFile("ArchiveTest." + desc.Ext);
				desc.Ar->LoadFromFile(loadFile);
			}

			GenericArchiveTest(*desc.Ar);

			if (type == EArchiveType::Saving)
			{
				File saveFile("ArchiveTest." + desc.Ext);
				desc.Ar->SaveToFile(saveFile);
			}
		}
	}

	void ArchiveTest()
	{
		TestArchives(EArchiveType::Saving);
		TestArchives(EArchiveType::Loading);

		//ionbreak();
	}
}
