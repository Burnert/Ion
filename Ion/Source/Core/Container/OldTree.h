#pragma once

#include "Core/CoreApi.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/MetaPointer.h"
#include "Core/Math/Math.h"

// @TODO: Ok this is really messy,
// I should have just made the nodes public
// and not bother with these wrapper classes
// Please refactor this

template<typename T, size_t ChunksInBlock = 256>
class TTree;

template<typename T>
struct TTreeElementNode;

template<typename T>
struct TTreeNodeBase
{
	using ElementType  = T;
	using NodeBaseType = TTreeNodeBase<T>;
	using NodeType     = TTreeElementNode<T>;
	using NodeArray    = TArray<NodeBaseType*>;

	/* The pointer to the parent node + IsElementNode flag */
	TMetaPointer<NodeBaseType> Parent;
	NodeArray Children;

	inline void InsertChild(NodeBaseType* node)
	{
		Children.emplace_back(node);
	}

	inline void RemoveChild(NodeBaseType* node)
	{
		Children.erase(std::find(Children.begin(), Children.end(), node));
	}

	inline bool HasChildren() const
	{
		return !Children.empty();
	}

	inline NodeBaseType* FindChildNode(const ElementType& element)
	{
		for (NodeBaseType* child : Children)
		{
			if (child->IsElementNode() && ((NodeType*)child)->Element == element)
			{
				return child;
			}
			// Recursive find
			NodeBaseType* child2 = child->FindChildNode(element);
			if (child2)
			{
				return child2;
			}
		}
		return nullptr;
	}

	inline bool IsElementNode() const
	{
		return Parent.GetMetaFlag<0>();
	}

	inline TTreeNodeBase(NodeBaseType* parent) :
		Parent(parent)
	{
	}
};

template<typename T>
struct TTreeElementNode : public TTreeNodeBase<T>
{
	ElementType Element;

	inline TTreeElementNode(NodeBaseType* parent, const ElementType& element) :
		TTreeNodeBase(parent),
		Element(element)
	{
		SetElementFlag();
	}

	inline TTreeElementNode(NodeBaseType* parent, ElementType&& element) noexcept :
		TTreeNodeBase(parent),
		Element(Move(element))
	{
		SetElementFlag();
	}

private:
	inline void SetElementFlag()
	{
		Parent.SetMetaFlag<0>(true);
	}
};

template<typename T>
class TTreeNodeRef
{
public:
	using ElementType  = T;
	using NodeRef      = TTreeNodeRef<T>;
	using NodeType     = TTreeElementNode<T>;
	using NodeBaseType = TTreeNodeBase<T>;
	using NodeArray    = TArray<NodeBaseType*>;

	inline bool IsValid() const
	{
		return m_NodePtr != nullptr;
	}

	inline ElementType* Element() const
	{
		if (m_NodePtr && m_NodePtr->IsElementNode())
			return &((NodeType*)m_NodePtr)->Element;

		return nullptr;
	}

	inline bool IsElementNode() const
	{
		return m_NodePtr && m_NodePtr->IsElementNode();
	}

	inline bool HasChildren() const
	{
		return m_NodePtr->HasChildren();
	}

	inline NodeArray& GetChildren()
	{
		return m_NodePtr->Children;
	}

	inline bool operator==(const NodeRef& other) const
	{
		return m_NodePtr == other.m_NodePtr;
	}

	inline bool operator!=(const NodeRef& other) const
	{
		return m_NodePtr != other.m_NodePtr;
	}

	inline operator bool() const
	{
		return IsValid();
	}

	// Creates an empty node
	inline TTreeNodeRef() :
		m_NodePtr(nullptr)
	{
	}

	inline TTreeNodeRef(NodeBaseType* node) :
		m_NodePtr(node)
	{
	}

	NodeBaseType* m_NodePtr;

	template<typename T, size_t ChunksInBlock>
	friend class TTree;
};

template<typename T>
class TTreeIterator
{
public:
	using ElementType  = T;
	using NodeType     = TTreeElementNode<ElementType>;
	using NodeBaseType = TTreeNodeBase<ElementType>;
	using NodeRef      = TTreeNodeRef<T>;
	using NodeArray    = TArray<NodeBaseType*>;
	using Iterator     = TTreeIterator<T>;

	struct NodeData
	{
		NodeBaseType* Node;
		size_t Index;
	};

	inline Iterator& Next()
	{
		if (m_NodeStack.empty())
			return *this;

		NodeBaseType* topNode = m_NodeStack.top();

		// Must be the root
		if (m_NodeStack.size() == 1)
		{
			m_NodeStack.push()
		}

		if (m_Node->HasChildren())
		{
			
		}
	}

	inline Iterator& operator++()
	{
		return Next();
	}

	inline Iterator operator++(int)
	{
		Iterator it = *this;
		Next();
		return it;
	}

	inline ElementType& operator*()
	{
		return ((NodeType*)m_Node)->Element;
	}

	inline operator bool() const
	{
		return (bool)m_Node;
	}

private:
	inline TTreeIterator(NodeBaseType& initialNode)
	{
		// Trace back the nodes
		TStack<NodeBaseType*> reverseStack;
		NodeBaseType* current = &initialNode;
		reverseStack.push(current);
		while (initialNode.Parent)
		{
			current = initialNode.Parent;
			reverseStack.push(current);
		}
		// Reverse the stack
		while (!reverseStack.empty())
		{
			m_NodeStack.push(reverseStack.top());
			reverseStack.pop();
		}
	}

private:
	TStack<NodeData> m_NodeStack;

	template<typename T, size_t ChunksInBlock>
	friend class TTree;
};

template<typename T, size_t ChunksInBlock>
class TTree
{
public:
	using Type             = TTree<T, ChunksInBlock>;
	using ElementType      = T;
	using NodeType         = TTreeElementNode<T>;
	using NodeBaseType     = TTreeNodeBase<T>;
	using NodeRef          = TTreeNodeRef<T>;
	using NodeArray        = TArray<NodeBaseType*>;
	using Iterator         = TTreeIterator<T>;
	using NodeAllocator    = TPoolAllocator<NodeType, ChunksInBlock>;

	/* Inserts an element at the root node. */
	inline NodeRef Insert(const ElementType& element)
	{
		return NodeRef(&InsertNode(element, m_RootNode));
	}

	/* Inserts an element at the root node. */
	inline NodeRef Insert(ElementType&& element)
	{
		return NodeRef(&InsertNode(Move(element), m_RootNode));
	}

	/* Inserts an element at the specified node (parent node).
	   If the NodeRef is invalid, it does not do anything */
	inline NodeRef Insert(const ElementType& element, NodeRef& atNode)
	{
		if (!atNode)
			return NodeRef();

		return NodeRef(&InsertNode(element, *atNode.m_NodePtr));
	}

	/* Inserts an element at the specified node (parent node).
	   If the NodeRef is invalid, it does not do anything */
	inline NodeRef Insert(ElementType&& element, NodeRef& atNode)
	{
		if (!atNode)
			return NodeRef();

		return NodeRef(&InsertNode(Move(element), *atNode.m_NodePtr));
	}

	/* Inserts an empty node at the specified node (parent node).
	   If the NodeRef is invalid, it does not do anything */
	inline NodeRef Insert(NodeRef& atNode)
	{
		if (!atNode)
			return NodeRef();

		return NodeRef(&InsertNode(*atNode.m_NodePtr));
	}

	/* Remove the specified node from the tree.
	   If bReparent is true, it does not delete its children,
	   but reparents them to the parent of the deleted node. */
	inline bool Remove(const NodeRef& nodeRef, bool bReparent = false)
	{
		// @TODO: Somehow check if the NodeRef is valid here

		if (!nodeRef)
			return false;

		NodeBaseType& node = *nodeRef.m_NodePtr;

		if (!bReparent)
		{
			RemoveRecursive(node);
			return true;
		}

		NodeBaseType* parent = node.Parent;
		if (!parent)
		{
			LOG_ERROR("The children cannot be reparented, because the node has no parent.");
			return false;
		}

		ReparentNodeChildren(node, *parent);

		DestroyNode(node);
		return true;
	}

	inline bool Remove(Iterator&& where)
	{
		if (!where)
			return false;

		
	}

	/* Changes the parent of a node.
	   If the newParent argument is nullptr, the node will be parented to the root node of the tree. */
	inline void Reparent(const NodeRef& node, const NodeRef& newParent)
	{
		ionassert(node.IsValid());

		NodeBaseType* parentNodePtr = newParent ? newParent.m_NodePtr : &m_RootNode;
		NodeBaseType* nodePtr = node.m_NodePtr;

		ReparentNode(*nodePtr, *parentNodePtr);
	}

	template<typename Lambda>
	inline void ForEachNodeDepthFirst(Lambda forEach)
	{
		static_assert(TIsBaseOfV<TFunction<void(NodeRef)>, Lambda>);

		ForEachNodeDepthFirstInternal<false>(m_RootNode, forEach);
	}

	template<typename Lambda>
	inline void ForEachElementDepthFirst(Lambda forEach)
	{
		static_assert(TIsBaseOfV<TFunction<void(NodeRef)>, Lambda>);

		ForEachNodeDepthFirstInternal<true>(m_RootNode, forEach);
	}

	/* Returns a node which contains the element.
	   If the element could not be found, it returns an invalid NodeRef. */
	inline NodeRef FindNode(const ElementType& element)
	{
		return NodeRef(m_RootNode.FindChildNode(element));
	}

	inline Type GetSubTree(const NodeRef& fromNode)
	{
		ionassert(false)
		// imp
	}

	inline NodeRef GetRootNode() const
	{
		return NodeRef(&m_RootNode);
	}

	inline Iterator Begin() const
	{
		return Iterator(m_RootNode);
	}

	inline void LogTree() const
	{
		LogTree([](ElementType& element)
		{
			return String(element);
		});
	}

	template<typename Lambda>
	inline void LogTree(Lambda toString) const
	{
		LOG_INFO("Tree: ---------------------");
		PrintNode(&m_RootNode, 0, toString);
	}

	/* Create a tree with no root element. */
	inline TTree() :
		m_RootNode(ConstructNode(nullptr))
	{
	}

	/* Create a tree with a root element. */
	inline TTree(const ElementType& rootElement) :
		m_RootNode(ConstructNode(nullptr, rootElement))
	{
	}

	/* Create a tree with a root element. */
	inline TTree(ElementType&& rootElement) :
		m_RootNode(ConstructNode(nullptr, Move(rootElement)))
	{
	}

private:
	inline NodeBaseType& InsertNode(const ElementType& element, NodeBaseType& atNode)
	{
		NodeBaseType& node = ConstructNode(&atNode, element);
		atNode.InsertChild(&node);
		return node;
	}

	inline NodeBaseType& InsertNode(ElementType&& element, NodeBaseType& atNode)
	{
		NodeBaseType& node = ConstructNode(&atNode, Move(element));
		atNode.InsertChild(&node);
		return node;
	}

	inline NodeBaseType& InsertNode(NodeBaseType& atNode)
	{
		NodeBaseType& node = ConstructNode(&atNode);
		atNode.InsertChild(&node);
		return node;
	}

	inline void RemoveRecursive(NodeBaseType& node)
	{
		// Remove the children first
		if (node.HasChildren())
		{
			for (NodeBaseType* child : node.Children)
			{
				RemoveRecursive(*child);
			}
		}

		NodeBaseType* parent = node.Parent;
		if (parent)
		{
			parent->RemoveChild(&node);
		}
		DestroyNode(node);
	}

	inline void ReparentNodeChildren(NodeBaseType& node, NodeBaseType& parent)
	{
		NodeArray& children = node.Children;
		for (NodeBaseType* child : children)
		{
			parent.InsertChild(child);
			child->Parent = &parent;
		}
		parent.RemoveChild(&node);
	}

	inline void ReparentNode(NodeBaseType& node, NodeBaseType& parent)
	{
		node.Parent->RemoveChild(&node);
		parent.InsertChild(&node);
		node.Parent = &parent;
	}

	template<bool bOnlyElementNodes, typename Lambda>
	inline void ForEachNodeDepthFirstInternal(NodeType& node, Lambda forEach)
	{
		if constexpr (bOnlyElementNodes)
		{
			if (node.IsElementNode())
			{
				forEach(NodeRef(node));
			}
		}
		if (node.HasChildren())
		{
			for (NodeType& child : node.Children)
			{
				ForEachNodeDepthFirstInternal<bOnlyElementNodes>(child, forEach);
			}
		}
	}

	// Constructs a node with no element
	inline NodeBaseType& ConstructNode(NodeBaseType* parent)
	{
		NodeBaseType& node = *m_NodeAllocator.Allocate();
		node = NodeBaseType(parent);
		return node;
	}

	inline NodeBaseType& ConstructNode(NodeBaseType* parent, const ElementType& element)
	{
		NodeType& node = *m_NodeAllocator.Allocate();
		node = NodeType(parent, element);
		return node;
	}

	inline NodeBaseType& ConstructNode(NodeBaseType* parent, ElementType&& element)
	{
		NodeType& node = *m_NodeAllocator.Allocate();
		node = NodeType(parent, Move(element));
		return node;
	}

	inline void DestroyNode(NodeBaseType& node)
	{
		m_NodeAllocator.Free(&node);
	}

	template<typename Lambda>
	inline void PrintNode(NodeBaseType* node, int32 level) const
	{
		PrintNode(node, level, [](ElementType& element) { return String(element); })
	}

	template<typename Lambda>
	inline void PrintNode(NodeBaseType* node, int32 level, Lambda toStringFunc) const
	{
		constexpr size_t _ShowMaxDepth = 10;
		constexpr const char _Indent[] = "> ";
		constexpr size_t _IndentLength = sizeof(_Indent) - 1;
		constexpr size_t _IndentBufferSize = _ShowMaxDepth * _IndentLength + 1;

		String prefix;
		for (int32 i = 0; i < Ion::Math::Min(level, (int32)_ShowMaxDepth); ++i)
		{
			prefix += _Indent;
		}
		// Write the depth as a number if the indent becomes too large
		if (level >= _ShowMaxDepth)
		{
			prefix += ToString(level) + " ";
		}

		if (node->IsElementNode())
		{
			ElementType& element = ((NodeType*)node)->Element;
			LOG_INFO("{0} {1}", prefix, toStringFunc(element));
		}
		else
		{
			String name = node == &m_RootNode ? "__ROOT_NODE__" : "__EMPTY_NODE__";
			LOG_INFO("{0} {1}", prefix, name);
		}

		if (node->HasChildren())
		{
			for (NodeBaseType* child : node->Children)
			{
				PrintNode((NodeType*)child, level + 1, toStringFunc);
			}
		}
	}

private:
	NodeAllocator m_NodeAllocator;
	NodeBaseType& m_RootNode;
};
