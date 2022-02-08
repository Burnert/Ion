#pragma once

namespace Ion
{
	class Entity;
	class WorldTreeNode;

	using WorldTreeNodeArray = TArray<WorldTreeNode>;

	class WorldTreeNode
	{
	public:
		WorldTreeNodeArray& GetChildren();
		const WorldTreeNodeArray& GetChildren() const;

		bool HasChildren() const;
		WorldTreeNode* FindChild(Entity* entity);
		void AddChild(Entity* entity);
		void RemoveChild(Entity* entity);

		Entity* GetEntity();
		const Entity* GetEntity() const;

	private:
		WorldTreeNode();

	private:
		Entity* m_Entity;
		WorldTreeNodeArray m_Children;

		friend class WorldTree;
	};

	class WorldTree
	{
	public:
		/* Returns nullptr, if the node hasn't been found. */
		WorldTreeNode* Find(Entity* entity);
		void Add(Entity* entity);
		void Remove(Entity* entity);

		const WorldTreeNodeArray& GetChildNodes() const;
		WorldTreeNode& GetRootNode();
		const WorldTreeNode& GetRootNode() const;

	private:
		WorldTree();

	private:
		WorldTreeNode m_RootNode;

		friend class World;
	};

	// Inline definitions --------------------------------------------------------

	// WorldTreeNode:

	inline WorldTreeNodeArray& WorldTreeNode::GetChildren()
	{
		return m_Children;
	}

	inline const WorldTreeNodeArray& WorldTreeNode::GetChildren() const
	{
		return m_Children;
	}

	inline bool WorldTreeNode::HasChildren() const
	{
		return !m_Children.empty();
	}

	inline WorldTreeNode* WorldTreeNode::FindChild(Entity* entity)
	{
		if (HasChildren())
		{
			// @TODO: Linear search will be kinda slow with a lot of entities
			for (WorldTreeNode& node : m_Children)
			{
				if (entity == node.m_Entity)
				{
					return &node;
				}
				// Recursive find
				if (WorldTreeNode* node2 = node.FindChild(entity))
				{
					return node2;
				}
			}
		}
		return nullptr;
	}

	inline void WorldTreeNode::AddChild(Entity* entity)
	{
		WorldTreeNode node { };
		node.m_Entity = entity;

		m_Children.emplace_back(Move(node));
	}

	inline void WorldTreeNode::RemoveChild(Entity* entity)
	{
		auto it = std::find_if(m_Children.begin(), m_Children.end(), [entity](WorldTreeNode& node)
		{
			return node.GetEntity() == entity;
		});

		if (it != m_Children.end())
		{
			m_Children.erase(it);
		}
	}

	inline Entity* WorldTreeNode::GetEntity()
	{
		return m_Entity;
	}

	inline const Entity* WorldTreeNode::GetEntity() const
	{
		return m_Entity;
	}

	inline WorldTreeNode::WorldTreeNode() :
		m_Entity(nullptr)
	{
	}

	// WorldTree:

	inline WorldTreeNode* WorldTree::Find(Entity* entity)
	{
		return m_RootNode.FindChild(entity);
	}

	inline void WorldTree::Add(Entity* entity)
	{
		m_RootNode.AddChild(entity);
	}

	inline void WorldTree::Remove(Entity* entity)
	{
		m_RootNode.RemoveChild(entity);
	}

	inline const WorldTreeNodeArray& WorldTree::GetChildNodes() const
	{
		return m_RootNode.GetChildren();
	}

	inline WorldTreeNode& WorldTree::GetRootNode()
	{
		return m_RootNode;
	}

	inline const WorldTreeNode& WorldTree::GetRootNode() const
	{
		return m_RootNode;
	}

	inline WorldTree::WorldTree() :
		m_RootNode({ })
	{
	}
}
