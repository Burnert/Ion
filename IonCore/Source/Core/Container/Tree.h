#pragma once

#include "Core/Base.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/MetaPointer.h"
#include "Core/Math/Math.h"

namespace Ion
{
	template<typename T, size_t NodesInBlock = 256>
	class TTreeNodeFactory;

	template<typename T, bool bFastNode = false /* Uses factory? */>
	struct TTreeNode
	{
		using ElementType = T;
		using NodeType    = TTreeNode<T, bFastNode>;
		using NodeArray   = TArray<NodeType*>;

		/**
		 * @brief Make a normal node (allocated using new)
		 * 
		 * @param element Node element
		 * @return Node reference 
		 */
		static inline TTreeNode<T, false>& Make(const ElementType& element)
		{
			return *(new TTreeNode<T, false>(element));
		}

		/**
		 * @brief Make a normal node (allocated using new)
		 * 
		 * @param element Node element
		 * @return Node reference 
		 */
		static inline TTreeNode<T, false>& Make(ElementType&& element)
		{
			return *(new TTreeNode<T, false>(Move(element)));
		}

		inline ElementType& Get()
		{
			return m_Element;
		}

		inline const ElementType& Get() const
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

		/**
		 * @brief Remove a node by reference.
		 *
		 * @details In case of Fast Nodes;
		 * Remember to destroy the node after removing
		 * it from the tree, if it's not needed anymore.
		 *
		 * @param node Child node reference
		 * @return Fast Node: Child node reference; Normal node: Child node copy
		 */
		inline NodeType& Remove(NodeType& node)
		{
			auto it = std::find(m_Children.begin(), m_Children.end(), &node);
			ionassert(it != m_Children.end(), "Cannot find the node.");
			m_Children.erase(it);
			node.m_Parent = nullptr;
			return node;
		}

		/**
		 * @brief Remove a node at the index.
		 * 
		 * @details In case of Fast Nodes;
		 * Remember to destroy the node after removing
		 * it from the tree, if it's not needed anymore.
		 * 
		 * @param at Child node index
		 * @return Fast Node: Child node reference; Normal node: Child node copy
		 */
		inline NodeType& Remove(size_t at)
		{
			ionassert(at < m_Children.size());

			NodeType* node = m_Children[at];
			m_Children.erase(m_Children.begin() + at);
			node->m_Parent = nullptr;
			return *node;
		}

		inline NodeType& RemoveFromParent(bool* bOutRemoved = nullptr)
		{
			if (m_Parent)
			{
				m_Parent->Remove(*this);
			}
			if (bOutRemoved)
				*bOutRemoved = m_Parent;
			return *this;
		}

		/**
		 * @brief Swap the nodes by their indices.
		 * 
		 * @param index1 First node index
		 * @param index2 Second node index
		 */
		inline void SwapNodes(size_t index1, size_t index2)
		{
			ionassert(index1 < m_Children.size());
			ionassert(index2 < m_Children.size());

			std::swap(m_Children[index1], m_Children[index2]);
		}

		template<typename Pred>
		inline void SortNodes(Pred compare)
		{
			std::sort(m_Children.begin(), m_Children.end(), [&compare](NodeType* left, NodeType* right) -> bool
			{
				return compare(left->m_Element, right->m_Element);
			});
		}

		/**
		 * @brief Finds a node by reference (don't pass in a copy)
		 * 
		 * @param node Node reference
		 * @return Child index
		 */
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

		/**
		 * @brief Finds a first node that satisfies the predicate, using depth first search.
		 * 
		 * @tparam Pred Lambda (ElementType&) -> bool
		 * @param pred Lambda
		 * @return Found node pointer, or nullptr
		 */
		template<typename Pred>
		inline NodeType* FindNodeRecursiveDF(Pred pred) const
		{
			for (NodeType* node : m_Children)
			{
				if (pred(node->Get()))
					return node;

				if (NodeType* node2 = node->FindNodeRecursiveDF(pred))
					return node2;
			}
			return nullptr;
		}

		/**
		 * @brief Finds all nodes that satisfy the predicate, using depth first search.
		 * 
		 * @tparam Pred Lambda (ElementType&) -> bool
		 * @param pred Lambda
		 * @return Array of all found node pointers
		 */
		template<typename Pred>
		inline TArray<NodeType*> FindAllNodesRecursiveDF(Pred pred) const
		{
			TArray<NodeType*> foundNodes;

			FindAllNodesRecursiveDF_Internal(foundNodes, pred);

			return foundNodes;
		}

		inline NodeType* FindNodeByElementRecursiveDF(const ElementType& element) const
		{
			return FindNodeRecursiveDF([&element](ElementType& el)
			{
				return el == element;
			});
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

		inline bool HasParent() const
		{
			return (bool)m_Parent;
		}

		~TTreeNode()
		{
			if constexpr (!bFastNode)
			{
				for (NodeType* node : m_Children)
				{
					delete node;
				}
			}
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

		template<typename Pred>
		inline void FindAllNodesRecursiveDF_Internal(TArray<NodeType*>& outNodes, Pred pred) const
		{
			for (NodeType* node : m_Children)
			{
				if (pred(node->Get()))
					outNodes.push_back(node);

				node->FindAllNodesRecursiveDF_Internal(outNodes, pred);
			}
		}

	private:
		NodeArray m_Children;
		NodeType* m_Parent;
		ElementType m_Element;

		template<typename U, size_t NodesInBlock>
		friend class TTreeNodeFactory;
		friend TFunction<bool(NodeType*, NodeType*)>;
	};

	template<typename T>
	using TFastTreeNode = TTreeNode<T, true>;

#if ION_DEBUG
#define _TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(ptr) m_Allocations_Debug.insert(ptr)
#define _TREENODEALLOC_ERASE_NODE_ALLOC_DEBUG(ptr) m_Allocations_Debug.erase(ptr)
#define _TREENODEALLOC_CHECK_NODE_ALLOC_DEBUG(ptr) m_Allocations_Debug.find(ptr) != m_Allocations_Debug.end()
#else
#define _TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(ptr)
#define _TREENODEALLOC_ERASE_NODE_ALLOC_DEBUG(ptr)
#define _TREENODEALLOC_CHECK_NODE_ALLOC_DEBUG(ptr)
#endif

	template<typename T, size_t NodesInBlock>
	class TTreeNodeFactory
	{
	public:
		using ElementType = T;
		using NodeType    = TFastTreeNode<T>;
		using NodeArray   = TArray<NodeType*>;

		inline NodeType& Create(const ElementType& element)
		{
			NodeType* nodePtr = m_NodeAllocator.Allocate();
			new(nodePtr) NodeType(element);
			_TREENODEALLOC_INSERT_NODE_ALLOC_DEBUG(nodePtr);
				return *nodePtr;
		}

		inline NodeType& Create(ElementType&& element)
		{
			NodeType* nodePtr = m_NodeAllocator.Allocate();
			new(nodePtr) NodeType(Move(element));
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
		TPoolAllocator<NodeType, NodesInBlock> m_NodeAllocator;
#if ION_DEBUG
		THashSet<void*> m_Allocations_Debug;
#endif
	};
}
