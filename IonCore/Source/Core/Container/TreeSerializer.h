#pragma once

#include "Tree.h"
#include "Core/Serialization/Archive.h"

namespace Ion
{
	template<typename TNode, typename TFactory = void>
	struct TTreeSerializer;

	template<typename TNode, typename TFactory>
	struct TTreeSerializer
	{
		static_assert(TIsFastTreeNode<TNode>::value);

		using TElement = typename TNode::ElementType;

		TTreeSerializer(TNode& node, TFactory& factory) :
			Tree(node),
			Factory(factory)
		{
		}

		TTreeSerializer(const TTreeSerializer&) = default;
		TTreeSerializer(TTreeSerializer&&) = default;

		TTreeNode<TElement, true>& Tree;
		TFactory& Factory;
	};

	template<typename TNode>
	struct TTreeSerializer<TNode, void>
	{
		static_assert(!TIsFastTreeNode<TNode>::value);

		using TElement = typename TNode::ElementType;

		TTreeSerializer(TNode& node) :
			Tree(node)
		{
		}

		TTreeSerializer(const TTreeSerializer&) = default;
		TTreeSerializer(TTreeSerializer&&) = default;

		TTreeNode<TElement, false>& Tree;
	};

	template<typename TNode, typename TFactory>
	FORCEINLINE Archive& operator&=(Archive& ar, const TTreeSerializer<TNode, TFactory>& treeSerializer)
	{
		ar &= TTreeSerializer<TNode, TFactory>(treeSerializer);
		return ar;
	}

	template<typename TNode, typename TFactory>
	FORCEINLINE Archive& operator&=(Archive& ar, TTreeSerializer<TNode, TFactory>&& treeSerializer)
	{
		using ElementType = typename TNode::ElementType;
		static constexpr bool bIsFastNode = TIsFastTreeNode<TNode>::value;

		// Make some checks first
		static_assert(!bIsFastNode || !std::is_void_v<TFactory>);

		// Function that serializes a node with its children
		// It does not actually serialize the nodes themselves, but the elements owned by them
		TFunction<void(TNode&)> LSerializeNode = [&](TNode& node)
		{
			// Get() returns a reference, not a copy. This is a valid read/write operation.
			ar &= node.Get();

			// Note: If the archive is loading, the children size will be 0 here...
			size_t childrenSize = node.GetChildrenSize();
			// ...but will get updated here.
			ar &= childrenSize;

			for (size_t n = 0; n < childrenSize; ++n)
			{
				TNode* childNode;

				if (ar.IsSaving())
				{
					auto children = node.GetChildren();
					childNode = children[n];
				}
				// Nodes have to be instantiated first when loading.
				else if (ar.IsLoading())
				{
					// Create a node with default constructed elements
					// (might not work in some cases, but I hope it won't matter).
					if constexpr (bIsFastNode)
					{
						childNode = &treeSerializer.Factory.Create(ElementType());
					}
					else
					{
						childNode = &TNode::Make(ElementType());
					}
					node.Insert(*childNode);
				}
				
				// Recursively serialize children
				LSerializeNode(*childNode);
			}
		};

		LSerializeNode(treeSerializer.Tree);

		return ar;
	}
}
