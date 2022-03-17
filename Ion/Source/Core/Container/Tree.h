#pragma once

#include "Core/CoreTypes.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/MetaPointer.h"
#include "Core/Math/Math.h"

template<typename T>
class TTreeNodeFactory;

template<typename T>
struct TTreeNode
{
	using ElementType = T;
	using NodeType    = TTreeNode<T>;
	using NodeArray   = TArray<NodeType*>;

	inline ElementType& Get()
	{
		return m_Element;
	}

	/* For the best performance, only insert nodes from the same factory. */
	inline NodeType& Insert(NodeType& node)
	{
		m_Children.push_back(&node);
		node.m_Parent = this;
		return node;
	}

	/* For the best performance, only insert nodes from the same factory.
	   Inserts a node at a specified index. */
	inline NodeType& Insert(NodeType& node, size_t at)
	{
		ionassert(at <= m_Children.size());
		m_Children.insert(m_Children.begin() + at, &node);
		node.m_Parent = this;
		return node;
	}

	/* For the best performance, only insert nodes from the same factory.
	   Inserts a node after the specified node.
	   If 'after' cannot be found or is nullptr,
	   inserts the node at the beginning.
	   Returns the node anyway. */
	inline NodeType& Insert(NodeType& node, NodeType* after)
	{
		int64 index = after ? FindIndex(*after) : -1;
		if (index != -1)
			return Insert(node, index);

		return node;
	}

	/* Remember to destroy the node after removing it from the tree, if it's not needed anymore. */
	inline NodeType& Remove(NodeType& node)
	{
		auto it = std::find(m_Children.begin(), m_Children.end(), &node);
		ionassert(it != m_Children.end(), "Cannot find the node.");
		m_Children.erase(it);
		node.m_Parent = nullptr;
		return node;
	}

	/* Remember to destroy the node after removing it from the tree, if it's not needed anymore. */
	inline NodeType& Remove(size_t at)
	{
		ionassert(at < m_Children.size());
		NodeType& node = *m_Children[at];
		m_Children.erase(m_Children.begin() + at);
		node.m_Parent = nullptr;
		return node;
	}

	inline NodeType& RemoveFromParent(bool* bOutRemoved = nullptr)
	{
		if (m_Parent)
		{
			m_Parent->Remove(*this);
			if (bOutRemoved)
				*bOutRemoved = true;
			return *this;
		}
		if (bOutRemoved)
			*bOutRemoved = false;
		return *this;
	}

	inline void SwapNodes(size_t index1, size_t index2)
	{
		ionassert(index1 < m_Children.size());
		ionassert(index2 < m_Children.size());
		NodeType* temp = m_Children[index1];
		m_Children[index1] = m_Children[index2];
		m_Children[index2] = temp;
	}

	template<typename Pred>
	inline void SortNodes(Pred compare)
	{
		std::sort(m_Children.begin(), m_Children.end(), [&compare](NodeType* left, NodeType* right) -> bool
		{
			return compare(left->m_Element, right->m_Element);
		});
	}

	inline int64 FindIndex(NodeType& node) const
	{
		auto it = std::find(m_Children.begin(), m_Children.end(), &node);
		if (it == m_Children.end())
			return -1;

		return it - m_Children.begin();
	}

	inline int64 FindIndex(const ElementType& element) const
	{
		auto it = std::find_if(m_Children.begin(), m_Children.end(), [](ElementType& e)
		{
			return e == element;
		});
		if (it == m_Children.end())
			return -1;

		return it - m_Children.begin();
	}

	template<typename Pred>
	inline NodeType* FindNodeRecursiveDF(Pred pred)
	{
		for (NodeType* node : m_Children)
		{
			if (pred(node->Element()))
				return node;

			if (NodeType* node2 = node->FindNodeRecursiveDF(pred))
				return node2;
		}
		return nullptr;
	}

	inline NodeType* FindNodeByElementRecursiveDF(const ElementType& element)
	{
		return FindNodeRecursiveDF([&element](ElementType& el)
		{
			return el == element;
		});
	}

	inline ElementType& Element()
	{
		return m_Element;
	}

	inline const ElementType& Element() const
	{
		return m_Element;
	}

	inline const NodeArray& GetChildren() const
	{
		return m_Children;
	}

	inline bool HasChildren() const
	{
		return !m_Children.empty();
	}

	inline size_t GetChildrenSize() const
	{
		return m_Children.size();
	}

	inline NodeType* GetParent() const
	{
		return m_Parent;
	}

private:
	inline TTreeNode(const ElementType& element) :
		m_Element(element),
		m_Parent(nullptr)
	{
	}

	inline TTreeNode(ElementType&& element) noexcept :
		m_Element(Move(element)),
		m_Parent(nullptr)
	{
	}

private:
	NodeArray m_Children;
	NodeType* m_Parent;
	ElementType m_Element;

	friend class TTreeNodeFactory<T>;
	friend TFunction<bool(NodeType*, NodeType*)>;
};

#if ION_DEBUG
#define _TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(ptr) m_Allocations_Debug.insert(ptr)
#define _TREENODEALLOC_ERASE_NODE_ALLOC_DEBUG(ptr) m_Allocations_Debug.erase(ptr)
#define _TREENODEALLOC_CHECK_NODE_ALLOC_DEBUG(ptr) m_Allocations_Debug.find(ptr) != m_Allocations_Debug.end()
#else
#define _TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(ptr)
#define _TREENODEALLOC_ERASE_NODE_ALLOC_DEBUG(ptr)
#define _TREENODEALLOC_CHECK_NODE_ALLOC_DEBUG(ptr)
#endif

template<typename T>
class TTreeNodeFactory
{
public:
	using ElementType = T;
	using NodeType    = TTreeNode<T>;
	using NodeArray   = TArray<NodeType*>;

	inline NodeType& Create(const ElementType& element)
	{
		NodeType* nodePtr = m_NodeAllocator.Allocate();
		*nodePtr = NodeType(element);
		_TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(nodePtr);
			return *nodePtr;
	}

	inline NodeType& Create(ElementType&& element)
	{
		NodeType* nodePtr = m_NodeAllocator.Allocate();
		*nodePtr = NodeType(Move(element));
		_TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(nodePtr);
		return *nodePtr;
	}

	inline void Destroy(NodeType& node)
	{
		ionassert(_TREENODEALLOC_CHECK_NODE_ALLOC_DEBUG(&node),
			"The node has not been created by this factory or the pointer is not a node.");
		_TREENODEALLOC_ERASE_NODE_ALLOC_DEBUG(&node);
		m_NodeAllocator.Free(&node);
	}

private:
	TPoolAllocator<NodeType> m_NodeAllocator;
#if ION_DEBUG
	THashSet<void*> m_Allocations_Debug;
#endif
};
