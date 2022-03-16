#include "IonPCH.h"

#include "WorldTree.h"
#include "Entity.h"

namespace Ion
{
	WorldTree::NodeRef WorldTree::Add(Entity* entity, Entity* parent)
	{
		auto parentIt = m_EntityNodeMap.find(parent);
		if (parentIt == m_EntityNodeMap.end())
		{
			LOG_ERROR("Cannot add the entity {0} to the World Tree.\nReason: Cannot find entity {1} in the World Tree.",
				entity->GetName(), parent->GetName());
			return NodeRef();
		}

		NodeRef atNode = parentIt->second;

		NodeRef node = m_EntityTree.Insert(entity, atNode);
		m_EntityNodeMap.emplace(entity, node);
		return node;
	}

	void WorldTree::Reparent(Entity* entity, Entity* parent)
	{
		if (m_EntityNodeMap.find(entity) == m_EntityNodeMap.end())
		{
			LOG_ERROR("Cannot reparent the entity {0}.\nReason: Cannot find entity {0} in the World Tree.",
				entity->GetName());
			return;
		}
		if (m_EntityNodeMap.find(parent) == m_EntityNodeMap.end())
		{
			LOG_ERROR("Cannot reparent the entity {0}.\nReason: Cannot find entity {1} in the World Tree.",
				entity->GetName(), parent->GetName());
			return;
		}

		NodeRef node = m_EntityNodeMap.at(entity);
		NodeRef parentNode = m_EntityNodeMap.at(parent);

		m_EntityTree.Reparent(node, parentNode);
	}

	void WorldTree::LogTree() const
	{
		m_EntityTree.LogTree([](Entity*& entity)
		{
			return entity->GetName();
		});
	}
}
