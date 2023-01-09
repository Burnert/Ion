#include "Core/CorePCH.h"

#include "Archive.h"
#include "BinaryArchive.h"
#include "YAMLArchive.h"
#include "Core/GUID/GUID.h"
#include "Core/Container/Tree.h"
#include "Core/Container/TreeSerializer.h"

namespace Ion::Test
{
#define ON_YAML if (YAMLArchive* pAsYaml = dynamic_cast<YAMLArchive*>(&archive))

	void GenericArchiveTest(Archive& archive)
	{
		// ----------------------------------------------------

		// GUID test

		ON_YAML pAsYaml->EnterNode("Guid");
		GUID guidTest = archive.IsSaving() ? GUID::FromString("54a6f55c-feaf-4aa9-87cd-cc9b487c31ef").Unwrap() : GUID::Zero;
		archive << guidTest;
		ionassert(guidTest == GUID::FromString("54a6f55c-feaf-4aa9-87cd-cc9b487c31ef").Unwrap());
		ON_YAML pAsYaml->ExitNode();

		// String test

		ON_YAML pAsYaml->EnterNode("S1");
		String stringTest1 = archive.IsSaving() ? "ABCDEFG_STRTEST" : EmptyString;
		archive << stringTest1;
		ionassert(stringTest1 == "ABCDEFG_STRTEST");
		ON_YAML pAsYaml->ExitNode();

		ON_YAML pAsYaml->EnterNode("S2");
		String stringTest2 = archive.IsSaving() ? "NextString_____" : EmptyString;
		archive << stringTest2;
		ionassert(stringTest2 == "NextString_____");
		ON_YAML pAsYaml->ExitNode();

		ON_YAML pAsYaml->EnterNode("S3");
		String stringTest3 = archive.IsSaving() ? "  String3      " : EmptyString;
		archive << stringTest3;
		ionassert(stringTest3 == "  String3      ");
		ON_YAML pAsYaml->ExitNode();

		ON_YAML pAsYaml->EnterNode("S4");
		const char utfString[] = u8"u8 简体中文";
		String stringTest4 = archive.IsSaving() ? utfString : EmptyString;
		archive << stringTest4;
		ionassert(stringTest4 == utfString);
		ON_YAML pAsYaml->ExitNode();

		// TArray test

		ON_YAML pAsYaml->EnterNode("Array");
		ON_YAML pAsYaml->BeginSeq();
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
		archive << arrayTest;
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
		ON_YAML pAsYaml->EndSeq();
		ON_YAML pAsYaml->ExitNode();

		ON_YAML return;

		// Tree test

		ON_YAML pAsYaml->EnterNode("Tree");
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
		ON_YAML pAsYaml->ExitNode();
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
