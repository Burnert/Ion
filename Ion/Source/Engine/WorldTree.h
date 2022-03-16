#pragma once

#include "Core/Container/Tree.h"

namespace Ion
{
	class ION_API WorldTree
	{
	public:
		using Tree    = TTree<Entity*>;
		using NodeRef = Tree::NodeRef;
		using NodeMap = THashMap<Entity*, NodeRef>;

		inline NodeRef GetRootNode() const
		{
			return m_EntityTree.GetRootNode();
		}

		NodeRef Find(Entity* entity);
		NodeRef Add(Entity* entity);
		NodeRef Add(Entity* entity, NodeRef& parent);
		NodeRef Add(Entity* entity, Entity* parent);
		bool Remove(Entity* entity);
		bool Remove(NodeRef& node);

		void Reparent(Entity* entity, Entity* parent);

		template<typename Lambda>
		void IterateDepthFirst(Lambda forEach);
		template<typename Lambda>
		void IterateDepthFirst(Lambda forEach) const;

		void LogTree() const;

	private:
		WorldTree();

	private:
		Tree m_EntityTree;
		NodeMap m_EntityNodeMap;

		friend class World;
	};

	// Inline definitions --------------------------------------------------------

	inline WorldTree::NodeRef WorldTree::Find(Entity* entity)
	{
		return m_EntityTree.FindNode(entity);
	}

	inline WorldTree::NodeRef WorldTree::Add(Entity* entity)
	{
		NodeRef node = m_EntityTree.Insert(entity);
		m_EntityNodeMap.emplace(entity, node);
		return node;
	}

	inline WorldTree::NodeRef WorldTree::Add(Entity* entity, NodeRef& parent)
	{
		NodeRef node = m_EntityTree.Insert(entity, parent);
		m_EntityNodeMap.emplace(entity, node);
		return node;
	}

	inline bool WorldTree::Remove(Entity* entity)
	{
		auto entityIt = m_EntityNodeMap.find(entity);
		if (entityIt != m_EntityNodeMap.end())
		{
			return m_EntityTree.Remove(entityIt->second, true);
		}
		return false;
	}

	inline bool WorldTree::Remove(NodeRef& node)
	{
		if (node)
		{
			return m_EntityTree.Remove(node, true);
		}
		return false;
	}

	template<typename Lambda>
	inline void WorldTree::IterateDepthFirst(Lambda forEach)
	{
		m_EntityTree.ForEachElementDepthFirst(forEach);
	}

	template<typename Lambda>
	inline void WorldTree::IterateDepthFirst(Lambda forEach) const
	{
		m_EntityTree.ForEachElementDepthFirst(forEach);
	}

	inline WorldTree::WorldTree() :
		m_EntityTree()
	{
		m_EntityNodeMap.emplace(nullptr, m_EntityTree.GetRootNode());
	}

//	class Entity;
//	class WorldTreeNode;
//
//	using WorldTreeNodeArray = TArray<WorldTreeNode>;
//
//	class WorldTreeNode
//	{
//	public:
//		WorldTreeNodeArray& GetChildren();
//		const WorldTreeNodeArray& GetChildren() const;
//
//		bool HasChildren() const;
//		WorldTreeNode* FindChild(Entity* entity);
//		void AddChild(Entity* entity);
//		void RemoveChild(Entity* entity);
//
//		Entity* GetEntity();
//		const Entity* GetEntity() const;
//
//	private:
//		WorldTreeNode();
//
//	private:
//		Entity* m_Entity;
//		WorldTreeNodeArray m_Children;
//
//		friend class WorldTree;
//	};
//
//	class WorldTree
//	{
//	public:
//		/* Returns nullptr, if the node hasn't been found. */
//		WorldTreeNode* Find(Entity* entity);
//		void Add(Entity* entity);
//		void Remove(Entity* entity);
//
//		const WorldTreeNodeArray& GetChildNodes() const;
//		WorldTreeNode& GetRootNode();
//		const WorldTreeNode& GetRootNode() const;
//
//	private:
//		WorldTree();
//
//	private:
//		WorldTreeNode m_RootNode;
//
//		friend class World;
//	};
//
//	// Inline definitions --------------------------------------------------------
//
//	// WorldTreeNode:
//
//	inline WorldTreeNodeArray& WorldTreeNode::GetChildren()
//	{
//		return m_Children;
//	}
//
//	inline const WorldTreeNodeArray& WorldTreeNode::GetChildren() const
//	{
//		return m_Children;
//	}
//
//	inline bool WorldTreeNode::HasChildren() const
//	{
//		return !m_Children.empty();
//	}
//
//	inline WorldTreeNode* WorldTreeNode::FindChild(Entity* entity)
//	{
//		TRACE_FUNCTION();
//
//		if (HasChildren())
//		{
//			// @TODO: Linear search will be kinda slow with a lot of entities
//			for (WorldTreeNode& node : m_Children)
//			{
//				if (entity == node.m_Entity)
//				{
//					return &node;
//				}
//				// Recursive find
//				if (WorldTreeNode* node2 = node.FindChild(entity))
//				{
//					return node2;
//				}
//			}
//		}
//		return nullptr;
//	}
//
//	inline void WorldTreeNode::AddChild(Entity* entity)
//	{
//		TRACE_FUNCTION();
//
//		WorldTreeNode node { };
//		node.m_Entity = entity;
//
//		m_Children.emplace_back(Move(node));
//	}
//
//	inline void WorldTreeNode::RemoveChild(Entity* entity)
//	{
//		TRACE_FUNCTION();
//
//		auto it = std::find_if(m_Children.begin(), m_Children.end(), [entity](WorldTreeNode& node)
//		{
//			return node.GetEntity() == entity;
//		});
//
//		if (it != m_Children.end())
//		{
//			m_Children.erase(it);
//		}
//	}
//
//	inline Entity* WorldTreeNode::GetEntity()
//	{
//		return m_Entity;
//	}
//
//	inline const Entity* WorldTreeNode::GetEntity() const
//	{
//		return m_Entity;
//	}
//
//	inline WorldTreeNode::WorldTreeNode() :
//		m_Entity(nullptr)
//	{
//	}
//
//	// WorldTree:
//
//	inline WorldTreeNode* WorldTree::Find(Entity* entity)
//	{
//		return m_RootNode.FindChild(entity);
//	}
//
//	inline void WorldTree::Add(Entity* entity)
//	{
//		m_RootNode.AddChild(entity);
//	}
//
//	inline void WorldTree::Remove(Entity* entity)
//	{
//		m_RootNode.RemoveChild(entity);
//	}
//
//	inline const WorldTreeNodeArray& WorldTree::GetChildNodes() const
//	{
//		return m_RootNode.GetChildren();
//	}
//
//	inline WorldTreeNode& WorldTree::GetRootNode()
//	{
//		return m_RootNode;
//	}
//
//	inline const WorldTreeNode& WorldTree::GetRootNode() const
//	{
//		return m_RootNode;
//	}
//
//	inline WorldTree::WorldTree() :
//		m_RootNode({ })
//	{
//	}
}
